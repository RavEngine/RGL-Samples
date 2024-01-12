#include "Common/ExampleFramework.hpp"
#include <RGL/Buffer.hpp>
#include <RGL/Texture.hpp>
#include <RGL/Sampler.hpp>
#include <RGL/Pipeline.hpp>
#include <RGL/RenderPass.hpp>
#include <RGL/CommandBuffer.hpp>

struct Cubemap : public ExampleFramework {

    RGLCommandBufferPtr commandBuffer;
    RGLBufferPtr vertexBuffer;
    RGLRenderPipelinePtr pipeline;
    RGLRenderPassPtr renderPass;

    struct ubo {
        glm::ivec2 screenDim;
    } ubo;

    void sampleinit(int argc, char** argv) final {
        vertexBuffer = device->CreateBuffer({
            {.VertexBuffer = true},
            sizeof(BasicObjects::ScreenTriangle::Vertex),
            BasicObjects::ScreenTriangle::vertices,
            RGL::BufferAccess::Private
        });
        vertexBuffer->SetBufferData(BasicObjects::ScreenTriangle::vertices);


        auto layout = device->CreatePipelineLayout({
            .bindings = {
               
            },
            .constants = {
                {ubo, 0, RGL::StageVisibility::Fragment}
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
                        .stride = sizeof(BasicObjects::ScreenTriangle::Vertex),
                    }},
                        .attributeDescs = {
                            {
                                .location = 0,
                                .binding = 0,
                                .offset = offsetof(BasicObjects::ScreenTriangle::Vertex,pos),
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
        //commandBuffer->SetFragmentSampler(mipSampler, 0);
        //commandBuffer->SetFragmentTexture(mipTexture->GetDefaultView(), 1);
        commandBuffer->Draw(3);

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
        renderPass.reset();
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