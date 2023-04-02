#include "Common/ExampleFramework.hpp"
#include <RGL/Pipeline.hpp>
#include <RGL/Buffer.hpp>
#include <RGL/CommandBuffer.hpp>
#include <RGL/Texture.hpp>
#include <RGL/Sampler.hpp>
#include <RGL/RenderPass.hpp>
#include <iostream>
#include <SDL.h>
#include <SDL_syswm.h>
#include <random>
#include <tiny_obj_loader.h>

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
            return std::bit_cast<uint32_t>(vertex.pos.x) ^ std::bit_cast<uint32_t>(vertex.pos.y) ^ std::bit_cast<uint32_t>(vertex.pos.z) ^ std::bit_cast<uint32_t>(vertex.normal.x) ^ std::bit_cast<uint32_t>(vertex.pos.y) ^ std::bit_cast<uint32_t>(vertex.normal.z) ^ std::bit_cast<uint32_t>(vertex.uv.x) ^ std::bit_cast<uint32_t>(vertex.uv.y);
        }
    };
}

struct Asteroids : public ExampleFramework {
    RGLRenderPipelinePtr planetRenderPipeline;
    RGLBufferPtr planetVertexBuffer, planetIndexBuffer, instanceDataBuffer, asteroidVertexBuffer, asteroidIndexBuffer;
    uint32_t ringStartIndex = 0, asteroidLod1StartIndex = 0, asteroidLod2StartIndex = 0,
    planetNIndicies = 0, ringNIndicies = 0;
        
    RGLCommandBufferPtr commandBuffer;
    RGLTexturePtr depthTexture;
    RGLRenderPassPtr renderPass;
    
   
    
    struct alignas(16) UniformBufferObject {
        glm::mat4 viewProj;
        glm::vec3 pos;
        float timeSinceStart;
        
    } ubo;
    
    const char* SampleName() {
        return "Asteroids";
    }
    void createDepthTexture()
    {
        // create the depth buffer
        depthTexture = device->CreateTexture({
            .usage = RGL::TextureUsage::DepthStencilAttachment,
            .aspect = RGL::TextureAspect::HasDepth,
            .width = (uint32_t)width,
            .height = (uint32_t)height,
            .format = RGL::TextureFormat::D32SFloat
            }
        );
    }
    void sampleinit(int argc, char** argv) final {
     
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
                        meshAttrib.vertices[item.vertex_index],
                        meshAttrib.vertices[item.vertex_index+1],
                        meshAttrib.vertices[item.vertex_index+2]
                    },
                    .normal = {
                        meshAttrib.normals[item.normal_index],
                        meshAttrib.normals[item.normal_index+1],
                        meshAttrib.normals[item.normal_index+2]
                    },
                    .uv{
                        meshAttrib.texcoords[item.texcoord_index],
                        meshAttrib.texcoords[item.texcoord_index+1],
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
            indexData1.insert(indexData1.begin(), indexData2.begin(), indexData2.end());
            return nextMeshStartIndex;
        };
        
        // load planet data
        {
            tinyobj::attrib_t planetAttrib, ringAttrib;
            std::vector<tinyobj::shape_t> planetShapes, ringShapes;
            tinyobj::LoadObj(&planetAttrib, &planetShapes, &materials, &warn, &err, "planet.obj");
            tinyobj::LoadObj(&ringAttrib, &ringShapes, &materials, &warn, &err, "ring.obj");
            
            auto planetData = getMeshBuffers(planetShapes[0], planetAttrib);
            planetNIndicies = planetData.second.size();
            auto ringData = getMeshBuffers(ringShapes[0], ringAttrib);
            ringNIndicies = ringData.second.size();
            // concatenate ring data into planet data
            ringStartIndex = concatenateBuffers(planetData.first, planetData.second, ringData.first, ringData.second);
            
            planetVertexBuffer = device->CreateBuffer({
                static_cast<uint32_t>(sizeof(objVertex) * planetData.first.size()),
                RGL::BufferConfig::Type::VertexBuffer,
                sizeof(objVertex),
                RGL::BufferAccess::Shared
            });
            planetVertexBuffer->SetBufferData(RGL::untyped_span{planetData.first.data(),planetData.first.size() * sizeof(objVertex)});
            planetIndexBuffer = device->CreateBuffer({
                static_cast<uint32_t>(sizeof(uint32_t) * planetData.second.size()),
                RGL::BufferConfig::Type::IndexBuffer,
                sizeof(uint32_t),
                RGL::BufferAccess::Shared
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
            asteroidLod1StartIndex = concatenateBuffers(asteroidTotal.first, asteroidTotal.second, asteroidNext.first, asteroidNext.second);
            
            asteroidNext = getMeshBuffers(astroiedLod2shapes[0],astroidLod2Attrib);
            asteroidLod2StartIndex = concatenateBuffers(asteroidTotal.first, asteroidTotal.second, asteroidNext.first, asteroidNext.second);
            
            asteroidVertexBuffer = device->CreateBuffer({
                static_cast<uint32_t>(sizeof(objVertex) * asteroidTotal.first.size()),
                RGL::BufferConfig::Type::VertexBuffer,
                sizeof(objVertex),
                RGL::BufferAccess::Shared
            });
            asteroidVertexBuffer->SetBufferData(RGL::untyped_span{asteroidTotal.first.data(),asteroidTotal.first.size() * sizeof(objVertex)});
            asteroidIndexBuffer = device->CreateBuffer({
                static_cast<uint32_t>(sizeof(uint32_t) * asteroidTotal.second.size()),
                RGL::BufferConfig::Type::IndexBuffer,
                sizeof(uint32_t),
                RGL::BufferAccess::Shared
            });
            asteroidIndexBuffer->SetBufferData(RGL::untyped_span{asteroidTotal.second.data(),asteroidTotal.second.size() * sizeof(uint32_t)});
        }
        
        createDepthTexture();
        
#pragma mark Create Pipelines

        auto vertexShaderLibrary = GetShader("vertex.vert");
        auto planetFragmentShader = GetShader("planet.frag");
        
        auto renderPipelineLayout = device->CreatePipelineLayout({
            .constants = {{ ubo, 0, RGL::StageVisibility::Vertex}}
        });

        // create a render pipeline
        planetRenderPipeline = device->CreateRenderPipeline({
            .stages = {
                {
                    .type = RGL::ShaderStageDesc::Type::Vertex,
                    .shaderModule = vertexShaderLibrary,
                },
                {
                    .type = RGL::ShaderStageDesc::Type::Fragment,
                    .shaderModule = planetFragmentShader,
                }
            },
            .vertexConfig = {
                .vertexBindinDesc = {
                    .binding = 0,
                    .stride = sizeof(objVertex),
                },
                .attributeDescs = {
                    {
                        .location = 0,
                        .binding = 0,
                        .offset = offsetof(BasicObjects::Cube::Vertex,pos),
                        .format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
                    },
                    {
                        .location = 1,
                        .binding = 0,
                        .offset = offsetof(BasicObjects::Cube::Vertex,normal),
                        .format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
                    },
                    {
                        .location = 2,
                        .binding = 0,
                        .offset = offsetof(BasicObjects::Cube::Vertex,uv),
                        .format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
                    }
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
        });
        
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
        renderPass->SetDepthAttachmentTexture(depthTexture.get());

        // create command buffer
        commandBuffer = commandQueue->CreateCommandBuffer();
        
        camera.position.z = 10;
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

        renderPass->SetAttachmentTexture(0, nextimg);

        commandBuffer->TransitionResource(nextimg, RGL::ResourceLayout::Undefined, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::TransitionPosition::Top);
        commandBuffer->BeginRendering(renderPass);

        commandBuffer->SetViewport({
                .width = static_cast<float>(nextImgSize.width),
                .height = static_cast<float>(nextImgSize.height),
            });
        commandBuffer->SetScissor({
                .extent = {nextImgSize.width, nextImgSize.height}
            });

        commandBuffer->BindPipeline(planetRenderPipeline);
        commandBuffer->SetVertexBytes(ubo, 0);
        
        // draw the planet
        commandBuffer->SetVertexBuffer(planetVertexBuffer);
        commandBuffer->SetIndexBuffer(planetIndexBuffer);
        commandBuffer->DrawIndexed(planetNIndicies);
        commandBuffer->DrawIndexed(ringNIndicies, {
            .firstIndex = ringStartIndex
        });

        commandBuffer->EndRendering();
        commandBuffer->TransitionResource(nextimg, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::ResourceLayout::Present, RGL::TransitionPosition::Bottom);
        commandBuffer->End();
        
        RGL::CommitConfig commitconfig{
            .signalFence = swapchainFence,
        };
        commandBuffer->Commit(commitconfig);

        swapchain->Present(presentConfig);
    }

    void onresize(int width, int height) final {
        createDepthTexture();
        renderPass->SetDepthAttachmentTexture(depthTexture.get());    // we recreated it so we need to reset it
    }

    void sampleshutdown() final {

        instanceDataBuffer.reset();
        renderPass.reset();
        depthTexture.reset();
        commandBuffer.reset();
        commandQueue.reset();

        planetVertexBuffer.reset();
        planetIndexBuffer.reset();

        planetRenderPipeline.reset();
    }
};

START_SAMPLE(Asteroids);
