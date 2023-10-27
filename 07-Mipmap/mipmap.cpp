#include "Common/ExampleFramework.hpp"

struct Mipmap : public ExampleFramework {
    
    struct alignas(16) UniformBufferObject {
        glm::mat4 viewProj;
        
    } ubo;
    
    RGLCommandBufferPtr commandBuffer;
    RGLBufferPtr vertexBuffer, indexBuffer;
    RGLTexturePtr depthTexture, mipTexture;
    RGLSamplerPtr mipSampler;
    RGLRenderPipelinePtr renderPipeline;
    RGLRenderPassPtr renderPass;
    
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
        
        mipTexture = device->CreateTexture({
            .usage = {.TransferDestination = true, .Sampled = true},
            .aspect = {.HasColor = true},
            .width = 4096,
            .height = 4096,
            .mipLevels = 4,
            .format = RGL::TextureFormat::RGBA16_Sfloat,
            .debugName = "Mip Texture"
            }
        );
        mipSampler = device->CreateSampler({});
        
        RGL::PipelineLayoutDescriptor layoutConfig{
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
            .constants = {{ ubo, 0, RGL::StageVisibility::Vertex}}
        };
        
        auto renderPipelineLayout = device->CreatePipelineLayout(layoutConfig);
        
        auto vertexShaderLibrary = GetShader("mipmap.vert");
        auto fragmentShaderLibrary = GetShader("mipmap.frag");
        
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
        
        commandBuffer = commandQueue->CreateCommandBuffer();
        camera.position.z = 10;
        camera.position.y = 0.3;
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

        renderPass->SetAttachmentTexture(0, nextimg);
        renderPass->SetDepthAttachmentTexture(depthTexture.get());
        
        commandBuffer->BeginRendering(renderPass);
        commandBuffer->SetViewport({
                .width = static_cast<float>(nextImgSize.width),
                .height = static_cast<float>(nextImgSize.height),
            });
        commandBuffer->SetScissor({
                .extent = {nextImgSize.width, nextImgSize.height}
            });
        
        commandBuffer->BindRenderPipeline(renderPipeline);
        commandBuffer->SetVertexBytes(ubo, 0);
        commandBuffer->SetVertexBuffer(vertexBuffer);
        commandBuffer->SetIndexBuffer(indexBuffer);
        commandBuffer->SetFragmentSampler(mipSampler, 0);
        commandBuffer->SetFragmentTexture(mipTexture.get(), 1);
        commandBuffer->DrawIndexed(std::size(BasicObjects::Quad::indices));

        commandBuffer->EndRendering();
        commandBuffer->End();
        
        RGL::CommitConfig commitconfig{
            .signalFence = swapchainFence,
        };
        commandBuffer->Commit(commitconfig);

        swapchain->Present(presentConfig);
    }
    
    void sampleshutdown() final {
        commandBuffer.reset();
        renderPass.reset();
        renderPipeline.reset();
        mipTexture.reset();
        mipSampler.reset();
        depthTexture.reset();
        indexBuffer.reset();
        vertexBuffer.reset();
    }
    
    void createDepthTexture()
    {
        // create the depth buffer
        depthTexture = device->CreateTexture({
            .usage = {.DepthStencilAttachment = true},
            .aspect = {.HasDepth = true},
            .width = (uint32_t)width,
            .height = (uint32_t)height,
            .format = RGL::TextureFormat::D32SFloat,
            .debugName = "Depth Texture"
            }
        );
    }
    
    const char* SampleName() final{
        return "Mipmap";
    }
    
    void onresize(int width, int height) final {
        createDepthTexture();
    }
};

START_SAMPLE(Mipmap);
