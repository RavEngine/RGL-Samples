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
    RGLRenderPipelinePtr renderPipeline;
    RGLBufferPtr planetVertexBuffer, planetIndexBuffer, instanceDataBuffer, asteroidVertexBuffer, asteroidIndexBuffer;
    uint32_t ringStartIndex = 0;
        
    RGLCommandBufferPtr commandBuffer;
    RGLTexturePtr depthTexture;
    RGLRenderPassPtr renderPass;
    
   
    
    struct alignas(16) UniformBufferObject {
        glm::mat4 viewProj;
        float timeSinceStart;
        
    } ubo;
    
    const char* SampleName() {
        return "Cubes";
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
        
        auto vertexShaderLibrary = GetShader("cubes.vert");
        auto fragmentShaderLibrary = GetShader("cubes.frag");
        
        tinyobj::attrib_t planetAttrib, ringAttrib, astroidLod0Attrib, astroidLod1Attrib, astroidLod2Attrib;
        std::vector<tinyobj::shape_t> planetShapes, ringShapes, astroiedLod0shapes, astroiedLod1shapes, astroiedLod2shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        tinyobj::LoadObj(&planetAttrib, &planetShapes, &materials, &warn, &err, "planet.obj");
        tinyobj::LoadObj(&ringAttrib, &ringShapes, &materials, &warn, &err, "ring.obj");
        tinyobj::LoadObj(&astroidLod0Attrib, &astroiedLod0shapes, &materials, &warn, &err, "asteroid_lod0.obj");
        tinyobj::LoadObj(&astroidLod1Attrib, &astroiedLod1shapes, &materials, &warn, &err, "asteroid_lod1.obj");
        tinyobj::LoadObj(&astroidLod2Attrib, &astroiedLod2shapes, &materials, &warn, &err, "asteroid_lod2.obj");
        
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
        auto planetData = getMeshBuffers(planetShapes[0], planetAttrib);
        auto ringData = getMeshBuffers(ringShapes[0], ringAttrib);
        // concatenate ring data into planet data
        ringStartIndex = planetData.second.size();
        const auto indexOffset = planetData.first.size();
        std::transform(ringData.second.begin(), ringData.second.end(), ringData.second.begin(), [indexOffset](uint32_t index){
            return index + indexOffset;
        });
        planetData.first.insert(planetData.first.end(), ringData.first.begin(), ringData.first.end());
        planetData.second.insert(planetData.second.begin(), ringData.second.begin(), ringData.second.end());

        planetVertexBuffer = device->CreateBuffer({
            static_cast<uint32_t>(sizeof(objVertex) * planetData.first.size()),
            RGL::BufferConfig::Type::VertexBuffer,
            sizeof(objVertex),
            RGL::BufferAccess::Shared
        });
        planetVertexBuffer->SetBufferData(RGL::untyped_span{planetData.first.data(),planetData.first.size()});
        planetIndexBuffer = device->CreateBuffer({
            static_cast<uint32_t>(sizeof(objVertex) * planetData.second.size()),
            RGL::BufferConfig::Type::IndexBuffer,
            sizeof(uint32_t),
            RGL::BufferAccess::Shared
        });
        planetIndexBuffer->SetBufferData(RGL::untyped_span{planetData.second.data(),planetData.second.size()});
        
        createDepthTexture();

        // create a pipeline layout
        // this describes what we *can* bind to the shader
        RGL::PipelineLayoutDescriptor layoutConfig{
            .bindings = {
                {
                    .binding = 0,
                    .type = decltype(layoutConfig)::LayoutBindingDesc::Type::CombinedImageSampler,
                    .descriptorCount = 1,
                    .stageFlags = decltype(layoutConfig)::LayoutBindingDesc::StageFlags::Fragment,
                },
                {
                    .binding = 1,
                    .type = decltype(layoutConfig)::LayoutBindingDesc::Type::SampledImage,
                    .descriptorCount = 1,
                    .stageFlags = decltype(layoutConfig)::LayoutBindingDesc::StageFlags::Fragment,
                },
                {
                    .binding = 2,
                    .type = decltype(layoutConfig)::LayoutBindingDesc::Type::StorageBuffer,
                    .descriptorCount = 1,
                    .stageFlags = decltype(layoutConfig)::LayoutBindingDesc::StageFlags::Vertex,
                },
            },
            .boundSamplers = {
            },
            .constants = {{ ubo, 0, RGL::StageVisibility::Vertex}}
        };
        auto renderPipelineLayout = device->CreatePipelineLayout(layoutConfig);

        // create a render pipeline
        renderPipeline = device->CreateRenderPipeline({
            .stages = {
                {
                    .type = RGL::ShaderStageDesc::Type::Vertex,
                    .shaderModule = vertexShaderLibrary,
                },
                {
                    .type = RGL::ShaderStageDesc::Type::Fragment,
                    .shaderModule = fragmentShaderLibrary,
                }
            },
            .vertexConfig = {
                .vertexBindinDesc = {
                    .binding = 0,
                    .stride = sizeof(BasicObjects::Cube::Vertex),
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
                    .clearColor = { 0.4f, 0.6f, 0.9f, 1.0f},
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

        commandBuffer->BindPipeline(renderPipeline);
        commandBuffer->SetVertexBytes(ubo, 0);
        commandBuffer->BindBuffer(instanceDataBuffer, 2);
        commandBuffer->SetVertexBuffer(planetVertexBuffer);
        commandBuffer->SetIndexBuffer(planetIndexBuffer);
        commandBuffer->DrawIndexed(std::size(BasicObjects::Cube::indices));

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

        renderPipeline.reset();
    }
};

START_SAMPLE(Asteroids);
