#include "Common/ExampleFramework.hpp"
#include <RGL/Buffer.hpp>
#include <RGL/Texture.hpp>
#include <RGL/Sampler.hpp>
#include <RGL/Pipeline.hpp>
#include <RGL/RenderPass.hpp>
#include <RGL/CommandBuffer.hpp>

struct Cubemap : public ExampleFramework {

    RGLCommandBufferPtr commandBuffer;
    RGLBufferPtr vertexBuffer, indBuffer;
    RGLRenderPipelinePtr pipeline;
    RGLRenderPassPtr renderPass;
    RGLTexturePtr cubemapTexture;
    RGLSamplerPtr sampler;

    struct ubo {
        glm::mat4 viewProj;
        float timeSinceStart;
    } ubo;

    void sampleinit(int argc, char** argv) final {
        vertexBuffer = device->CreateBuffer({
            {.VertexBuffer = true},
            sizeof(BasicObjects::Cube::Vertex),
            BasicObjects::Cube::vertices,
            RGL::BufferAccess::Private
        });
        vertexBuffer->SetBufferData(BasicObjects::Cube::vertices);
        
        indBuffer = device->CreateBuffer({
            {.IndexBuffer = true},
            sizeof(BasicObjects::Cube::indices[0]),
            BasicObjects::Cube::indices,
            RGL::BufferAccess::Private
        });
        indBuffer->SetBufferData(BasicObjects::Cube::indices);


        auto layout = device->CreatePipelineLayout({
            .bindings = {
                 {
                    .binding = 0,
                    .type = RGL::BindingType::Sampler,
                    .stageFlags = RGL::BindingVisibility::Fragment,
                },
                {
                    .binding = 1,
                    .type = RGL::BindingType::SampledImage,
                    .stageFlags = RGL::BindingVisibility::Fragment,
                },
            },
            .constants = {
                {ubo, 0, RGL::StageVisibility::Vertex}
            }
        });

        pipeline = device->CreateRenderPipeline({
            .stages = {
                {
                    .type = RGL::ShaderStageDesc::Type::Vertex,
                    .shaderModule = GetShader("cubemap.vert"),
                },
                {
                    .type = RGL::ShaderStageDesc::Type::Fragment,
                    .shaderModule = GetShader("cubemap.frag"),
                }
            },
            .vertexConfig = {
                    .vertexBindings = {{
                        .binding = 0,
                        .stride = sizeof(BasicObjects::Cube::Vertex),
                    }},
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
                .windingOrder = RGL::WindingOrder::Clockwise,
            },
            .colorBlendConfig{
                .attachments = {
                    {
                        .format = RGL::TextureFormat::BGRA8_Unorm    // specify attachment data
                    }
                }
            },
            .pipelineLayout = layout,
        });

        renderPass = RGL::CreateRenderPass({
            .attachments = {
                {
                    .format = RGL::TextureFormat::BGRA8_Unorm,
                    .loadOp = RGL::LoadAccessOperation::Clear,
                    .storeOp = RGL::StoreAccessOperation::Store,
                    .clearColor = { 0.4f, 0.6f, 0.9f, 1.0f},
                },
            },
        });

        // create command buffer
        commandBuffer = commandQueue->CreateCommandBuffer();

        sampler = device->CreateSampler({});

        // create cubemap
        cubemapTexture = device->CreateTexture({
            .usage = {.TransferDestination = true, .Sampled = true},
            .aspect = {.HasColor = true},
            .width = 1024,
            .height = 1024,
            .arrayLayers = 6,
            .format = RGL::TextureFormat::BGRA8_Unorm,
            .initialLayout = RGL::ResourceLayout::Undefined,
            .isCubemap = true,
            .debugName = "Cubemap"
        });

        // upload cubemap data
        constexpr const char* const sides[] = {
            "right.jpg",
            "left.jpg",
            "top.jpg",
            "bottom.jpg",
            "front.jpg",
            "back.jpg"
        };
        
        auto tempCmdBuffer = commandQueue->CreateCommandBuffer();
        tempCmdBuffer->Begin();
        const auto dim = cubemapTexture->GetSize();
        uint8_t i = 0;

        // buffers must be valid for the lifetime of the commandbuffer
        std::array<RGLBufferPtr, std::size(sides)> imagedataBuffers;

        for (const auto side : sides) {
            auto data = LoadImage(side);
            auto tmpbuffer = device->CreateBuffer({
                static_cast<uint32_t>(data.bytes.size_bytes),
                {.StorageBuffer = true},
                sizeof(char),
                RGL::BufferAccess::Private,
                {.Transfersource = true}
            });
            tmpbuffer->SetBufferData(data.bytes);
            tempCmdBuffer->CopyBufferToTexture(tmpbuffer, data.bytes.size_bytes, {
                .view = cubemapTexture->GetDefaultView(),
                .destLoc = {
                    .offset = {0,0},
                    .extent = {dim.width, dim.height}
                },
                .arrayLayer = i,
            });
            imagedataBuffers[i] = tmpbuffer;
            i++;
        }
        tempCmdBuffer->End();
        tempCmdBuffer->Commit({});
        tempCmdBuffer->BlockUntilCompleted();
    }

    void tick() final {
        
        RGL::SwapchainPresentConfig presentConfig{
        };

        swapchain->GetNextImage(&presentConfig.imageIndex);
        swapchainFence->Wait();
        swapchainFence->Reset();
        commandBuffer->Reset();
        commandBuffer->Begin();
        auto nextimg = swapchain->ImageAtIndex(presentConfig.imageIndex);
        auto nextImgSize = nextimg->GetSize();
        
        // automatically spin
        auto viewMat = glm::toMat4(glm::quat(glm::vec3(0, getTimeSeconds() * 0.5 ,0)));
        ubo.viewProj = camera.GenerateProjectionMatrix(nextImgSize.width, nextImgSize.height) * viewMat;

        renderPass->SetAttachmentTexture(0, nextimg->GetDefaultView());

        commandBuffer->BeginRendering(renderPass);
        commandBuffer->SetViewport({
                .width = static_cast<float>(nextImgSize.width),
                .height = static_cast<float>(nextImgSize.height),
            });
        commandBuffer->SetScissor({
                .extent = {nextImgSize.width, nextImgSize.height}
            });

        commandBuffer->BindRenderPipeline(pipeline);
        commandBuffer->SetVertexBytes(ubo, 0);
        commandBuffer->SetVertexBuffer(vertexBuffer);
        commandBuffer->SetIndexBuffer(indBuffer);
        commandBuffer->SetFragmentSampler(sampler, 0);
        commandBuffer->SetFragmentTexture(cubemapTexture->GetDefaultView(), 1);
        commandBuffer->DrawIndexed(std::size(BasicObjects::Cube::indices));

        commandBuffer->EndRendering();
        commandBuffer->End();

        RGL::CommitConfig commitconfig{
            .signalFence = swapchainFence,
        };
        commandBuffer->Commit(commitconfig);
        commandBuffer->BlockUntilCompleted();

        swapchain->Present(presentConfig);
    }

    void sampleshutdown() final {
        sampler.reset();
        cubemapTexture.reset();
        renderPass.reset();
        indBuffer.reset();
        vertexBuffer.reset();
        pipeline.reset();
        commandBuffer.reset();
    }

    const char* SampleName() final {
        return "Cubemap";
    }

    void onresize(int width, int height) final {
        
    }
};
START_SAMPLE(Cubemap);
