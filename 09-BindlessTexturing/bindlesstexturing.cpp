#include "Common/ExampleFramework.hpp"
#include <RGL/Buffer.hpp>
#include <RGL/Texture.hpp>
#include <RGL/Sampler.hpp>
#include <RGL/Pipeline.hpp>
#include <RGL/RenderPass.hpp>
#include <RGL/CommandBuffer.hpp>


struct BindlessTexturing : public ExampleFramework {

    struct {
        glm::mat4 viewProj;
    } ubo;

    struct {
        glm::vec4 color;
        glm::ivec2 dim;
    } comp_ubo;

    RGLCommandBufferPtr commandBuffer;
    RGLBufferPtr vertexBuffer, indexBuffer;
    RGLTexturePtr depthTexture, tx1, tx2, tx3;
    RGLSamplerPtr sampler;
    RGLRenderPipelinePtr renderPipeline;
    RGLRenderPassPtr renderPass;

    constexpr static auto num_mips = 4;
    constexpr static auto img_size = 1024;
    constexpr static glm::vec4 colors[] = {
        {195 / 255.f,88 / 255.f,49 / 255.f, 1},
        {96 / 255.f,111 / 255.f,140 / 255.f, 1},
        {88 / 255.f,114 / 255.f,70 / 255.f, 1},
        {34 / 255.f,230 / 255.f,202 / 255.f, 1},
    };

    void sampleinit(int argc, char** argv) final {
        vertexBuffer = device->CreateBuffer({
            {.VertexBuffer = true},
            sizeof(BasicObjects::Quad::Vertex),
            BasicObjects::Quad::vertices,
            RGL::BufferAccess::Private
            });
        vertexBuffer->SetBufferData(BasicObjects::Quad::vertices);

        indexBuffer = device->CreateBuffer({
            {.IndexBuffer = true},
            sizeof(BasicObjects::Quad::indices[0]),
            BasicObjects::Quad::indices,
            RGL::BufferAccess::Private
            });
        indexBuffer->SetBufferData(BasicObjects::Quad::indices);

        createDepthTexture();


        // set sampler to trilinear filtering
        sampler = device->CreateSampler({ });

        auto renderPipelineLayout = device->CreatePipelineLayout({
            .bindings = {
                {
                    .binding = 0,
                    .type = RGL::BindingType::Sampler,
                    .stageFlags = RGL::BindingVisibility::Fragment,
                },
                {
                    .binding = 1,
                    .count = 512,
                    .type = RGL::BindingType::SampledImage,
                    .stageFlags = RGL::BindingVisibility::Fragment,
                },
            },
            .constants = {{ ubo, 0, RGL::StageVisibility::Vertex}}
            });

        auto vertexShaderLibrary = GetShader("btex.vert");
        auto fragmentShaderLibrary = GetShader("btex.frag");

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
                .vertexBindings = {{
                    .binding = 0,
                    .stride = sizeof(BasicObjects::Quad::Vertex),
                }},
                .attributeDescs = {
                    {
                        .location = 0,
                        .binding = 0,
                        .offset = offsetof(BasicObjects::Quad::Vertex,position),
                        .format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
                    },
                    {
                        .location = 1,
                        .binding = 0,
                        .offset = offsetof(BasicObjects::Quad::Vertex,uv),
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

        auto computePipelineLayout = device->CreatePipelineLayout({
            .bindings = {
                {
                    .binding = 0,
                    .type = RGL::BindingType::StorageImage,
                    .stageFlags = RGL::BindingVisibility::Compute,
                    .writable = true
                },
            },
            .constants = {
                {{ comp_ubo, 0, RGL::StageVisibility::Compute}}
            }
            });

        commandBuffer = commandQueue->CreateCommandBuffer();
        camera.position.z = 10;
        camera.position.y = 0.3;

        auto loadTx = [this](const std::string_view name, RGLTexturePtr& tx) {
            auto imagedata = LoadImage(name);

            tx = device->CreateTextureWithData(RGL::TextureConfig{
                .usage = {.TransferDestination = true, .Sampled = true},
                .aspect = {.HasColor = true},
                .width = imagedata.width,
                .height = imagedata.height,
                .format = RGL::TextureFormat::RGBA8_Unorm
                },
                imagedata.bytes
            );
        };
        loadTx("tx29.png",tx1);
        loadTx("tx34.png",tx2);
        loadTx("tx39.png",tx3);
       
    }

    void tick() final {
        ubo.viewProj = camera.GenerateViewProjMatrix(width, height);

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
        renderPass->SetDepthAttachmentTexture(depthTexture->GetDefaultView());

        commandBuffer->BeginRendering(renderPass);
        commandBuffer->SetViewport({
                .width = static_cast<float>(nextImgSize.width),
                .height = static_cast<float>(nextImgSize.height),
            });
        commandBuffer->SetScissor({
                .extent = {nextImgSize.width, nextImgSize.height}
            });

        auto heapStart = device->GetGlobalBindlessTextureHeap();

        commandBuffer->BindRenderPipeline(renderPipeline);
        commandBuffer->SetVertexBytes(ubo, 0);
        commandBuffer->SetVertexBuffer(vertexBuffer);
        commandBuffer->SetIndexBuffer(indexBuffer);
        commandBuffer->SetFragmentSampler(sampler,0);
        commandBuffer->SetFragmentTexture(heapStart, 1);        // expose all the textures
        commandBuffer->DrawIndexed(std::size(BasicObjects::Quad::indices), {.nInstances = 3});

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
        tx1.reset();
        tx2.reset();
        tx3.reset();
        renderPass.reset();
        depthTexture.reset();
        indexBuffer.reset();
        vertexBuffer.reset();
        renderPipeline.reset();
        commandBuffer.reset();
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
            .format = RGL::TextureFormat::D32SFloat,
            .debugName = "Depth Texture"
            }
        );
    }

    const char* SampleName() final {
        return "Mipmap";
    }

    void onresize(int width, int height) final {
        createDepthTexture();
    }
};

START_SAMPLE(BindlessTexturing);
