#include "Common/ExampleFramework.hpp"
#include "shared.h"
#include <RGL/Pipeline.hpp>
#include <RGL/Buffer.hpp>
#include <RGL/CommandBuffer.hpp>
#include <RGL/Texture.hpp>
#include <RGL/Sampler.hpp>
#include <RGL/RenderPass.hpp>
#include <iostream>
#include <random>
#include <bit>
#include <tiny_obj_loader.h>
#include <unordered_map>

struct objVertex{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    bool operator==(const objVertex& other) const{
        return pos == other.pos && normal == other.normal && uv == other.uv;
    }
};

namespace std{
    template<>
    struct hash<objVertex>{
        size_t operator()(const objVertex& vertex) const{
            auto px = reinterpret_cast<const uint32_t*>(&vertex.pos.x);
            auto py = reinterpret_cast<const uint32_t*>(&vertex.pos.y);
            auto pz = reinterpret_cast<const uint32_t*>(&vertex.pos.z);
            auto nx = reinterpret_cast<const uint32_t*>(&vertex.normal.x);
            auto ny = reinterpret_cast<const uint32_t*>(&vertex.normal.y);
            auto nz = reinterpret_cast<const uint32_t*>(&vertex.normal.z);
            
            return *px ^ *py ^ *pz ^ *nx ^ *ny ^ *nz;
        }
    };
}

struct Asteroids : public ExampleFramework {
    RGLRenderPipelinePtr planetRenderPipeline, ringRenderPipeline, asteroidRenderPipeline;
    RGLComputePipelinePtr lodPipeline;
    RGLBufferPtr planetVertexBuffer, planetIndexBuffer, asteroidVertexBuffer, asteroidIndexBuffer, indirectBuffer, perInstanceAttributeBuffer;
    uint32_t ringStartIndex = 0, planetNIndices = 0, ringNIndices = 0;
        
    RGLCommandBufferPtr commandBuffer;
    RGLTexturePtr depthTexture;
    RGLRenderPassPtr renderPass;
    
    struct alignas(16) UniformBufferObject {
        glm::mat4 viewProj;
        glm::vec3 pos;
        float timeSinceStart;
        uint32_t asteroidLod1StartIndex = 0, asteroidLod2StartIndex = 0, asteroidTotalIndices = 0;
    } ubo;
    
    struct ArgumentBufferEntry{
        uint32_t drawId;
    };
    
    const char* SampleName() {
        return "Asteroids";
    }
    void createDepthTexture()
    {
        // create the depth buffer
        depthTexture = device->CreateTexture({
            .usage = {.DepthStencilAttachment = true},
            .aspect = {.HasDepth = true},
            .width = (uint32_t)width,
            .height = (uint32_t)height,
            .optimizedClearValue = {1},
            .format = RGL::TextureFormat::D32SFloat
            }
        );
    }
    void sampleinit(int argc, char** argv) final {
        
        indirectBuffer = device->CreateBuffer({
            nAsteroids,
            {.StorageBuffer = true, .IndirectBuffer = true},
            sizeof(RGL::IndirectIndexedCommand),
            RGL::BufferAccess::Private,
            {.Writable = true}
        });

        perInstanceAttributeBuffer = device->CreateBuffer({
           nAsteroids,
            {.StorageBuffer = true, .VertexBuffer = true},
           sizeof(uint32_t),
           RGL::BufferAccess::Private,
            {.Writable = true}
        });
        
#pragma mark Load Meshes
        auto getMeshBuffers = [](const tinyobj::shape_t& meshShape, const tinyobj::attrib_t& meshAttrib){
            uint32_t totalIndices = 0;
            std::unordered_map<objVertex, uint32_t> createdVertices;
            std::vector<objVertex> meshVertices;
            std::vector<uint32_t> meshIndices;
            for(const auto& item : meshShape.mesh.indices){
                uint32_t index = totalIndices;
                objVertex vert{
                    .pos = {
                        meshAttrib.vertices[3 * item.vertex_index],
                        meshAttrib.vertices[3 * item.vertex_index+1],
                        meshAttrib.vertices[3 * item.vertex_index+2]
                    },
                    .normal = {
                        meshAttrib.normals[3 * item.normal_index],
                        meshAttrib.normals[3 * item.normal_index+1],
                        meshAttrib.normals[3 * item.normal_index+2]
                    },
                    .uv{
                        meshAttrib.texcoords[2 * item.texcoord_index],
                        meshAttrib.texcoords[2 * item.texcoord_index+1],
                    }
                };
                // does this vert already exist?
                if (createdVertices.contains(vert)){
                    index = createdVertices.at(vert);
                }
                else{
                    // add it
                    createdVertices[vert] = totalIndices++;
                    meshVertices.push_back(vert);
                }
                meshIndices.push_back(index);
            }
            return std::make_pair(meshVertices, meshIndices);
        };
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        auto concatenateBuffers = [](auto& vertexData1, auto& indexData1, auto& vertexData2, auto& indexData2){
            const auto nextMeshStartIndex = indexData1.size();
            const auto indexOffset = vertexData1.size();
            std::transform(indexData2.begin(), indexData2.end(), indexData2.begin(), [indexOffset](uint32_t index){
                return index + indexOffset;
            });
            vertexData1.insert(vertexData1.end(), vertexData2.begin(), vertexData2.end());
            indexData1.insert(indexData1.end(), indexData2.begin(), indexData2.end());
            return nextMeshStartIndex;
        };
        
        // load planet data
        {
            tinyobj::attrib_t planetAttrib, ringAttrib;
            std::vector<tinyobj::shape_t> planetShapes, ringShapes;
            tinyobj::LoadObj(&planetAttrib, &planetShapes, &materials, &warn, &err, "planet.obj");
            tinyobj::LoadObj(&ringAttrib, &ringShapes, &materials, &warn, &err, "ring.obj");
            
            auto planetData = getMeshBuffers(planetShapes[0], planetAttrib);
            planetNIndices = planetData.second.size();
            auto ringData = getMeshBuffers(ringShapes[0], ringAttrib);
            ringNIndices = ringData.second.size();
            // concatenate ring data into planet data
            ringStartIndex = concatenateBuffers(planetData.first, planetData.second, ringData.first, ringData.second);
            
            planetVertexBuffer = device->CreateBuffer({
                uint32_t(planetData.first.size()),
                {.VertexBuffer = true},
                sizeof(objVertex),
                RGL::BufferAccess::Private
            });
            planetVertexBuffer->SetBufferData(RGL::untyped_span{planetData.first.data(),planetData.first.size() * sizeof(objVertex)});
            planetIndexBuffer = device->CreateBuffer({
                uint32_t(planetData.second.size()),
                {.IndexBuffer = true},
                sizeof(uint32_t),
                RGL::BufferAccess::Private
            });
            planetIndexBuffer->SetBufferData(RGL::untyped_span{planetData.second.data(),planetData.second.size() * sizeof(uint32_t)});
        }
        
        // load asteroids
        {
            tinyobj::attrib_t astroidLod0Attrib, astroidLod1Attrib, astroidLod2Attrib;
            std::vector<tinyobj::shape_t> astroiedLod0shapes, astroiedLod1shapes, astroiedLod2shapes;
            tinyobj::LoadObj(&astroidLod0Attrib, &astroiedLod0shapes, &materials, &warn, &err, "asteroid_lod0.obj");
            tinyobj::LoadObj(&astroidLod1Attrib, &astroiedLod1shapes, &materials, &warn, &err, "asteroid_lod1.obj");
            tinyobj::LoadObj(&astroidLod2Attrib, &astroiedLod2shapes, &materials, &warn, &err, "asteroid_lod2.obj");
            auto asteroidTotal = getMeshBuffers(astroiedLod0shapes[0], astroidLod0Attrib);
            
            auto asteroidNext = getMeshBuffers(astroiedLod1shapes[0],astroidLod1Attrib);
            ubo.asteroidLod1StartIndex = concatenateBuffers(asteroidTotal.first, asteroidTotal.second, asteroidNext.first, asteroidNext.second);
            
            asteroidNext = getMeshBuffers(astroiedLod2shapes[0],astroidLod2Attrib);
            ubo.asteroidLod2StartIndex = concatenateBuffers(asteroidTotal.first, asteroidTotal.second, asteroidNext.first, asteroidNext.second);
            
            asteroidVertexBuffer = device->CreateBuffer({
                uint32_t(asteroidTotal.first.size()),
                {.VertexBuffer = true},
                sizeof(objVertex),
                RGL::BufferAccess::Private
            });
            asteroidVertexBuffer->SetBufferData(RGL::untyped_span{asteroidTotal.first.data(),asteroidTotal.first.size() * sizeof(objVertex)});
            asteroidIndexBuffer = device->CreateBuffer({
                uint32_t(asteroidTotal.second.size()),
                {.IndexBuffer = true},
                sizeof(uint32_t),
                RGL::BufferAccess::Private
            });
            asteroidIndexBuffer->SetBufferData(RGL::untyped_span{asteroidTotal.second.data(),asteroidTotal.second.size() * sizeof(uint32_t)});
            
            ubo.asteroidTotalIndices = asteroidTotal.second.size();
        }
        
        createDepthTexture();
        
#pragma mark Create Pipelines

        auto vertexShaderLibrary = GetShader("vertex.vert");
        
        auto renderPipelineLayout = device->CreatePipelineLayout({
            .constants = {{ ubo, 0, RGL::StageVisibility{RGL::StageVisibility::Vertex | RGL::StageVisibility::Fragment}}},
        });
        
        auto createPipelineDescriptor = [this,&renderPipelineLayout](RGLShaderLibraryPtr vertexShader, RGLShaderLibraryPtr fragmentShader){
            return RGL::RenderPipelineDescriptor{
                .stages = {
                    {
                        .type = RGL::ShaderStageDesc::Type::Vertex,
                        .shaderModule = vertexShader,
                    },
                    {
                        .type = RGL::ShaderStageDesc::Type::Fragment,
                        .shaderModule = fragmentShader,
                    }
                },
                .vertexConfig = {
                    .vertexBindings = {
                        {
                            .binding = 0,
                            .stride = sizeof(objVertex),
                        },
                    },
                    .attributeDescs = {
                        {
                            .location = 0,
                            .binding = 0,
                            .offset = offsetof(objVertex,pos),
                            .format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
                        },
                        {
                            .location = 1,
                            .binding = 0,
                            .offset = offsetof(objVertex,normal),
                            .format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
                        },
                        {
                            .location = 2,
                            .binding = 0,
                            .offset = offsetof(objVertex,uv),
                            .format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
                        },                       
                    }
                },
                .inputAssembly = {
                    .topology = RGL::PrimitiveTopology::TriangleList,
                },
                .viewport = {
                    .width = (float)width,
                    .height = (float)height
                },
                .scissor = {
                    .extent = {width, height}
                },
                .rasterizerConfig = {
                    .windingOrder = RGL::WindingOrder::Counterclockwise,
                },
                .colorBlendConfig{
                    .attachments = {
                        {
                            .format = RGL::TextureFormat::BGRA8_Unorm    // specify attachment data
                        }
                    }
                },
                .depthStencilConfig = {
                    .depthFormat = RGL::TextureFormat::D32SFloat,
                    .depthTestEnabled = true,
                    .depthWriteEnabled = true,
                    .depthFunction = RGL::DepthCompareFunction::Less,
                },
                .pipelineLayout = renderPipelineLayout,
            };
        };

        // create a render pipeline
        planetRenderPipeline = device->CreateRenderPipeline(createPipelineDescriptor(vertexShaderLibrary, GetShader("planet.frag")));
        {
            auto desc = createPipelineDescriptor(vertexShaderLibrary, GetShader("ring.frag"));
            desc.rasterizerConfig.cullMode = RGL::CullMode::None;   // we want to draw both sides here
            ringRenderPipeline = device->CreateRenderPipeline(desc);
        }
        {
            auto desc = createPipelineDescriptor(GetShader("asteroid.vert"), GetShader("asteroid.frag"));
            desc.vertexConfig.vertexBindings.push_back({
                            .binding = 1,
                            .stride = sizeof(uint32_t),
                            .inputRate = RGL::InputRate::Instance,
                });
            desc.vertexConfig.attributeDescs.push_back(
                {
                           .location = 3,
                           .binding = 1,
                           .offset = 0,
                           .format = RGL::VertexAttributeFormat::R32_Uint,
                }
            );
            asteroidRenderPipeline = device->CreateRenderPipeline(desc);
        }
        
        renderPass = RGL::CreateRenderPass({
            .attachments = {
                {
                    .format = RGL::TextureFormat::BGRA8_Unorm,
                    .loadOp = RGL::LoadAccessOperation::Clear,
                    .storeOp = RGL::StoreAccessOperation::Store,
                    .clearColor = {0,0,0, 1.0f},
                }
            },
            .depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
                .format = RGL::TextureFormat::D32SFloat,
                .loadOp = RGL::LoadAccessOperation::Clear,
                .storeOp = RGL::StoreAccessOperation::Store,
                .clearColor = {1,1,1,1}
            }
        });

        // the depth texture is not swapchained so we can set it once
        renderPass->SetDepthAttachmentTexture(depthTexture->GetDefaultView());

        // create command buffer
        commandBuffer = commandQueue->CreateCommandBuffer();

        // create the compute pipeline for use in deciding LOD

        auto lodPipelineLayout = device->CreatePipelineLayout({
            .bindings = {
                {
                    .binding = 2,
                    .type = RGL::BindingType::StorageBuffer,
                    .stageFlags = RGL::BindingVisibility::Compute,
                    .writable = true
                },
                {
                    .binding = 3,
                    .type = RGL::BindingType::StorageBuffer,
                    .stageFlags = RGL::BindingVisibility::Compute,
                    .writable = true
                },
            },
            .constants = {{ ubo, 0, RGL::StageVisibility::Compute}}
        });
        lodPipeline = device->CreateComputePipeline({
            .stage = {
                .type = RGL::ShaderStageDesc::Type::Compute,
                .shaderModule = GetShader("asteroid.comp")
            },
            .pipelineLayout = lodPipelineLayout
        });
        
        camera.position.z = 50;
        camera.farClip = 1000;
    }
    void tick() final {
        ubo.viewProj = camera.GenerateViewProjMatrix(width, height);
        ubo.timeSinceStart = getTimeSeconds();
        
        RGL::SwapchainPresentConfig presentConfig{
        };

        swapchain->GetNextImage(&presentConfig.imageIndex);
        swapchainFence->Wait();
        swapchainFence->Reset();
        commandBuffer->Reset();
        commandBuffer->Begin();
        auto nextimg = swapchain->ImageAtIndex(presentConfig.imageIndex);
        auto nextImgSize = nextimg->GetSize();

        renderPass->SetAttachmentTexture(0, nextimg->GetDefaultView());
        
        commandBuffer->BeginCompute(lodPipeline);
        commandBuffer->SetComputeBytes(ubo,0);
        commandBuffer->BindComputeBuffer(indirectBuffer, 2);
        commandBuffer->BindComputeBuffer(perInstanceAttributeBuffer, 3);
        commandBuffer->DispatchCompute(nAsteroids, 1, 1);
        commandBuffer->EndCompute();

        commandBuffer->BeginRendering(renderPass);

        commandBuffer->SetViewport({
                .width = static_cast<float>(nextImgSize.width),
                .height = static_cast<float>(nextImgSize.height),
            });
        commandBuffer->SetScissor({
                .extent = {nextImgSize.width, nextImgSize.height}
            });

        // draw planet and ring
        commandBuffer->BindRenderPipeline(planetRenderPipeline);
        commandBuffer->SetVertexBytes(ubo, 0);
        
        // draw the planet
        commandBuffer->SetFragmentBytes(ubo, 0);
        commandBuffer->SetVertexBuffer(planetVertexBuffer);
        commandBuffer->SetIndexBuffer(planetIndexBuffer);
        commandBuffer->DrawIndexed(planetNIndices);
        
        // draw ring
        commandBuffer->BindRenderPipeline(ringRenderPipeline);
        commandBuffer->SetVertexBytes(ubo, 0);
        commandBuffer->SetVertexBuffer(planetVertexBuffer);
        commandBuffer->SetIndexBuffer(planetIndexBuffer);
        commandBuffer->DrawIndexed(ringNIndices, {
            .firstIndex = ringStartIndex
        });
        
        
        // draw asteroids
        commandBuffer->BindRenderPipeline(asteroidRenderPipeline);
        commandBuffer->SetVertexBytes(ubo, 0);
        commandBuffer->SetVertexBuffer(asteroidVertexBuffer);
        commandBuffer->SetVertexBuffer(perInstanceAttributeBuffer, { .bindingPosition = 1 });
        commandBuffer->SetIndexBuffer(asteroidIndexBuffer);
        commandBuffer->ExecuteIndirectIndexed({
            .indirectBuffer = indirectBuffer,
            .nDraws = nAsteroids,
        });

        commandBuffer->EndRendering();
        commandBuffer->End();
        
        RGL::CommitConfig commitconfig{
            .signalFence = swapchainFence,
        };
        commandBuffer->Commit(commitconfig);
        commandBuffer->BlockUntilCompleted();

        swapchain->Present(presentConfig);
    }

    void onresize(int width, int height) final {
        createDepthTexture();
        renderPass->SetDepthAttachmentTexture(depthTexture->GetDefaultView());    // we recreated it so we need to reset it
    }

    void sampleshutdown() final {

        renderPass.reset();
        depthTexture.reset();
        commandBuffer.reset();

        planetVertexBuffer.reset();
        planetIndexBuffer.reset();
        perInstanceAttributeBuffer.reset();
        asteroidVertexBuffer.reset();
        asteroidIndexBuffer.reset();
        indirectBuffer.reset();

        planetRenderPipeline.reset();
        asteroidRenderPipeline.reset();
        ringRenderPipeline.reset();
        lodPipeline.reset();
    }
};

START_SAMPLE(Asteroids);
