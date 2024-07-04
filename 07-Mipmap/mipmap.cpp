#include "Common/ExampleFramework.hpp"
#include <RGL/Buffer.hpp>
#include <RGL/Texture.hpp>
#include <RGL/Sampler.hpp>
#include <RGL/Pipeline.hpp>
#include <RGL/RenderPass.hpp>
#include <RGL/CommandBuffer.hpp>

struct Mipmap : public ExampleFramework {
    
    struct {
        glm::mat4 viewProj;
    } ubo;
    
    struct {
        glm::vec4 color;
        glm::ivec2 dim;
    } comp_ubo;
    
    RGLCommandBufferPtr commandBuffer;
    RGLBufferPtr vertexBuffer, indexBuffer;
    RGLTexturePtr depthTexture, mipTexture;
    RGLSamplerPtr mipSampler;
    RGLRenderPipelinePtr renderPipeline;
    RGLComputePipelinePtr mipPipeline;
    RGLRenderPassPtr renderPass;
    
    constexpr static auto num_mips = 4;
    constexpr static auto img_size = 1024;
    constexpr static glm::vec4 colors[] = {
        {195/255.f,88/255.f,49/255.f, 1},
        {96/255.f,111/255.f,140/255.f, 1},
        {88/255.f,114/255.f,70/255.f, 1},
        {34/255.f,230/255.f,202/255.f, 1},
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
        
        mipTexture = device->CreateTexture({
            .usage = {.TransferDestination = true, .Sampled = true, .Storage = true},
            .aspect = {.HasColor = true},
            .width = img_size,
            .height = img_size,
            .mipLevels = num_mips,
            .format = RGL::TextureFormat::RGBA16_Sfloat,
            .debugName = "Mip Texture"
            }
        );
        // set sampler to trilinear filtering
        mipSampler = device->CreateSampler({
            .minFilter = RGL::MinMagFilterMode::Nearest,
            .magFilter = RGL::MinMagFilterMode::Nearest,
            .mipFilter = RGL::MipFilterMode::Nearest,
        });
                
        auto renderPipelineLayout = device->CreatePipelineLayout({
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
        });
        
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
        mipPipeline = device->CreateComputePipeline({
            .stage = {
                .type = RGL::ShaderStageDesc::Type::Compute,
                .shaderModule = GetShader("mipmap_colorset.comp")
            },
            .pipelineLayout = computePipelineLayout
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

        renderPass->SetAttachmentTexture(0, nextimg->GetDefaultView());
        renderPass->SetDepthAttachmentTexture(depthTexture->GetDefaultView());
        
        commandBuffer->BeginCompute(mipPipeline);
        float current_width = img_size;
        for(int i = 0; i < num_mips; i++){
            auto view = mipTexture->GetViewForMip(i);
            int x = 0;
            comp_ubo.color = colors[i];
            comp_ubo.dim = {current_width, current_width};
            commandBuffer->SetComputeTexture(view, 0);
            commandBuffer->SetComputeBytes(comp_ubo, 0);
            commandBuffer->DispatchCompute(std::ceil(current_width / 32), std::ceil(current_width / 32), 1, 32, 32, 1);
            current_width /= 2;
        }
        commandBuffer->EndCompute();
        
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
        commandBuffer->SetFragmentTexture(mipTexture->GetDefaultView(), 1);
        commandBuffer->DrawIndexed(std::size(BasicObjects::Quad::indices));

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
        mipTexture.reset();
        mipSampler.reset();
        depthTexture.reset();
        indexBuffer.reset();
        vertexBuffer.reset();
        renderPipeline.reset();
        mipPipeline.reset();
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
    
    const char* SampleName() final{
        return "Mipmap";
    }
    
    void onresize(int width, int height) final {
        createDepthTexture();
    }
};

START_SAMPLE(Mipmap);
