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
#undef CreateSemaphore
#undef LoadImage

constexpr static uint32_t nCubes = 36;
static float cubeSpinSpeeds [nCubes]{0};

struct Deferred : public ExampleFramework {
    RGLPipelineLayoutPtr deferredRenderPipelineLayout, finalRenderPipelineLayout;
    RGLRenderPipelinePtr deferredRenderPipeline, dirLightRenderPipeline, finalRenderPipeline;
    RGLBufferPtr vertexBuffer, indexBuffer, screenTriVerts;
        
    RGLCommandBufferPtr commandBuffer;
    RGLTexturePtr depthTexture, colorTexture, normalTexture, positionTexture, lightingTexture;
    RGLSamplerPtr textureSampler;
    RGLRenderPassPtr deferredRenderPass, dirLightRenderPass, finalRenderPass;
    
    constexpr static RGL::TextureFormat
        posTexFormat = RGL::TextureFormat::RGBA32_Float,
        normalTexFormat = RGL::TextureFormat::RGBA16_Unorm,
        colorTexFormat = RGL::TextureFormat::RGBA16_Unorm;

    
    struct alignas(16) UniformBufferObject {
        glm::mat4 viewProj;
        float timeSinceStart;
        
    } ubo;
    
    const char* SampleName() {
        return "Deferred";
    }
    void updateGBuffers()
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
        colorTexture = device->CreateTexture({
            .usage = RGL::TextureUsage::ColorAttachment,
            .aspect = RGL::TextureAspect::HasColor,
            .width = (uint32_t)width,
            .height = (uint32_t)height,
            .format = colorTexFormat
            }
        );
        normalTexture = device->CreateTexture({
            .usage = RGL::TextureUsage::ColorAttachment,
            .aspect = RGL::TextureAspect::HasColor,
            .width = (uint32_t)width,
            .height = (uint32_t)height,
            .format = normalTexFormat
            }
        );
        positionTexture = device->CreateTexture({
            .usage = RGL::TextureUsage::ColorAttachment,
            .aspect = RGL::TextureAspect::HasColor,
            .width = (uint32_t)width,
            .height = (uint32_t)height,
            .format = posTexFormat
            }
        );
        lightingTexture = device->CreateTexture({
            .usage = RGL::TextureUsage::ColorAttachment,
            .aspect = RGL::TextureAspect::HasColor,
            .width = (uint32_t)width,
            .height = (uint32_t)height,
            .format = colorTexFormat
            }
        );
    }
    void sampleinit(int argc, char** argv) final {
        
        auto deferredVertexShader = GetShader("deferred.vert");
        auto deferredFragmentShader = GetShader("deferred.frag");
        
        auto screenTriVertexShader = GetShader("screentri.vert");
        auto dirLightFragmentShader = GetShader("dirlight.frag");
        auto finalFragmentShader = GetShader("final.frag");
        
        vertexBuffer = device->CreateBuffer({
            RGL::BufferConfig::Type::VertexBuffer,
            sizeof(BasicObjects::Cube::Vertex),
            BasicObjects::Cube::vertices,
        });
        vertexBuffer->SetBufferData(BasicObjects::Cube::vertices);
        
        screenTriVerts = device->CreateBuffer({
            RGL::BufferConfig::Type::VertexBuffer,
            sizeof(BasicObjects::ScreenTriangle::Vertex),
            BasicObjects::ScreenTriangle::vertices
        });
        screenTriVerts->SetBufferData(BasicObjects::ScreenTriangle::vertices);
        
        // seed the buffer with random values
        std::random_device rd;
        std::mt19937 mt{rd()};
        std::uniform_real_distribution<> distr(-5, 5);
        for(auto& value : cubeSpinSpeeds){
            value = distr(mt);
        }
        
        indexBuffer = device->CreateBuffer({
            RGL::BufferConfig::Type::IndexBuffer,
            sizeof(BasicObjects::Cube::indices[0]),
            BasicObjects::Cube::indices,
        });
        indexBuffer->SetBufferData(BasicObjects::Cube::indices);

        updateGBuffers();

        textureSampler = device->CreateSampler({});

        // create a pipeline layout
        // this describes what we *can* bind to the shader
        deferredRenderPipelineLayout = device->CreatePipelineLayout({
            .constants = {{ ubo, 0}}
        });
        
        finalRenderPipelineLayout = device->CreatePipelineLayout({
            .bindings = {
                {
                    .binding = 0,
                    .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::CombinedImageSampler,
                    .descriptorCount = 1,
                    .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
                },
                {
                    .binding = 1,
                    .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::SampledImage,
                    .descriptorCount = 1,
                    .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
                },
            },
            .boundSamplers = {
                textureSampler
            },
        });
    
        // create render pipelines
        deferredRenderPipeline = device->CreateRenderPipeline({
            .stages = {
                {
                    .type = RGL::ShaderStageDesc::Type::Vertex,
                    .shaderModule = deferredVertexShader,
                },
                {
                    .type = RGL::ShaderStageDesc::Type::Fragment,
                    .shaderModule = deferredFragmentShader,
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
                            .format = colorTexFormat
                        },
                        {
                            .format = normalTexFormat
                        },
                        {
                            .format = posTexFormat
                        }
                    }
                },
                .depthStencilConfig = {
                    .depthFormat = RGL::TextureFormat::D32SFloat,
                    .depthTestEnabled = true,
                    .depthWriteEnabled = true,
                    .depthFunction = RGL::DepthCompareFunction::Less,
                },
                .pipelineLayout = deferredRenderPipelineLayout,
        });
        
        finalRenderPipeline = device->CreateRenderPipeline({
            .stages = {
                {
                    .type = RGL::ShaderStageDesc::Type::Vertex,
                    .shaderModule = screenTriVertexShader,
                },
                {
                    .type = RGL::ShaderStageDesc::Type::Fragment,
                    .shaderModule = finalFragmentShader,
                }
            },
                .vertexConfig = {
                    .vertexBindinDesc = {
                        .binding = 0,
                        .stride = sizeof(BasicObjects::ScreenTriangle::Vertex),
                    },
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
                .pipelineLayout = finalRenderPipelineLayout,
        });
        
        dirLightRenderPipeline = device->CreateRenderPipeline({
            .stages = {
                {
                    .type = RGL::ShaderStageDesc::Type::Vertex,
                    .shaderModule = screenTriVertexShader,
                },
                {
                    .type = RGL::ShaderStageDesc::Type::Fragment,
                    .shaderModule = dirLightFragmentShader,
                }
            },
                .vertexConfig = {
                    .vertexBindinDesc = {
                        .binding = 0,
                        .stride = sizeof(BasicObjects::ScreenTriangle::Vertex),
                    },
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
                            .format = colorTexFormat    // specify attachment data
                        }
                    }
                },
                .pipelineLayout = finalRenderPipelineLayout,
        });
        
        deferredRenderPass = RGL::CreateRenderPass({
            .attachments = {
                {
                    .format = colorTexFormat,
                    .loadOp = RGL::LoadAccessOperation::Clear,
                    .storeOp = RGL::StoreAccessOperation::Store,
                    .clearColor = { 0.4f, 0.6f, 0.9f, 1.0f},
                    .shouldTransition = false
                },
                {
                    .format = normalTexFormat,
                    .loadOp = RGL::LoadAccessOperation::Clear,
                    .storeOp = RGL::StoreAccessOperation::Store,
                    .shouldTransition = false
                },
                {
                    .format = posTexFormat,
                    .loadOp = RGL::LoadAccessOperation::Clear,
                    .storeOp = RGL::StoreAccessOperation::Store,
                    .shouldTransition = false
                }

            },
            .depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
                .format = RGL::TextureFormat::D32SFloat,
                .loadOp = RGL::LoadAccessOperation::Clear,
                .storeOp = RGL::StoreAccessOperation::Store,
                .clearColor = {1,1,1,1}
            }
        });
        
        dirLightRenderPass = RGL::CreateRenderPass({
            .attachments = {
                {
                    .format = colorTexFormat,
                    .loadOp = RGL::LoadAccessOperation::Clear,
                    .storeOp = RGL::StoreAccessOperation::Store,
                    .shouldTransition = false
                }
            },
        });
        
        finalRenderPass = RGL::CreateRenderPass({
            .attachments = {
                {
                    .format = RGL::TextureFormat::BGRA8_Unorm,
                    .loadOp = RGL::LoadAccessOperation::Clear,
                    .storeOp = RGL::StoreAccessOperation::Store,
                    .clearColor = { 0.4f, 0.6f, 0.9f, 1.0f},
                    .shouldTransition = true        // for swapchain images
                }
            },
        });

        // the depth texture is not swapchained so we can set it once
        deferredRenderPass->SetDepthAttachmentTexture(depthTexture.get());
        deferredRenderPass->SetAttachmentTexture(0, colorTexture.get());
        deferredRenderPass->SetAttachmentTexture(1, normalTexture.get());
        deferredRenderPass->SetAttachmentTexture(2, positionTexture.get());

        dirLightRenderPass->SetAttachmentTexture(0, lightingTexture.get());

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
        
        // first do the deferred pass
        commandBuffer->BeginRendering(deferredRenderPass);
        commandBuffer->BindPipeline(deferredRenderPipeline);
        commandBuffer->SetViewport({
            .width = static_cast<float>(nextImgSize.width),
            .height = static_cast<float>(nextImgSize.height),
        });
        commandBuffer->SetScissor({
            .extent = {nextImgSize.width, nextImgSize.height}
        });
        commandBuffer->SetVertexBytes(ubo, 0);
        commandBuffer->SetVertexBuffer(vertexBuffer);
        commandBuffer->SetIndexBuffer(indexBuffer);
        commandBuffer->DrawIndexed(std::size(BasicObjects::Cube::indices), {
            .nInstances = nCubes
        });
        commandBuffer->EndRendering();
        
        // then do the lighting pass
        commandBuffer->BeginRendering(dirLightRenderPass);
        commandBuffer->BindPipeline(dirLightRenderPipeline);
        commandBuffer->SetViewport({
            .width = static_cast<float>(nextImgSize.width),
            .height = static_cast<float>(nextImgSize.height),
        });
        commandBuffer->SetScissor({
            .extent = {nextImgSize.width, nextImgSize.height}
        });
        commandBuffer->SetVertexBuffer(screenTriVerts);
        commandBuffer->Draw(std::size(BasicObjects::ScreenTriangle::vertices));
        commandBuffer->EndRendering();

        // next do the final render
        finalRenderPass->SetAttachmentTexture(0, nextimg);

        commandBuffer->BeginRendering(finalRenderPass);

        commandBuffer->SetViewport({
                .width = static_cast<float>(nextImgSize.width),
                .height = static_cast<float>(nextImgSize.height),
            });
        commandBuffer->SetScissor({
                .extent = {nextImgSize.width, nextImgSize.height}
            });

        commandBuffer->BindPipeline(finalRenderPipeline);
        commandBuffer->SetVertexBuffer(screenTriVerts);
        commandBuffer->Draw(std::size(BasicObjects::ScreenTriangle::vertices));

        commandBuffer->EndRendering();
        commandBuffer->End();
        
        RGL::CommitConfig commitconfig{
            .signalFence = swapchainFence,
        };
        commandBuffer->Commit(commitconfig);

        swapchain->Present(presentConfig);
    }

    void onresize(int width, int height) final {
        updateGBuffers();
        deferredRenderPass->SetDepthAttachmentTexture(depthTexture.get());    // we recreated it so we need to reset it
        deferredRenderPass->SetAttachmentTexture(0, colorTexture.get());
        deferredRenderPass->SetAttachmentTexture(1, normalTexture.get());
        deferredRenderPass->SetAttachmentTexture(2, positionTexture.get());
        
        dirLightRenderPass->SetAttachmentTexture(0, lightingTexture.get());
    }

    void sampleshutdown() final {

        finalRenderPass.reset();
        finalRenderPipelineLayout.reset();
        dirLightRenderPipeline.reset();
        dirLightRenderPass.reset();
        finalRenderPipeline.reset();
        screenTriVerts.reset();
        deferredRenderPass.reset();
        depthTexture.reset();
        colorTexture.reset();
        positionTexture.reset();
        normalTexture.reset();
        lightingTexture.reset();
        textureSampler.reset();
        commandBuffer.reset();
        commandQueue.reset();

        vertexBuffer.reset();
        indexBuffer.reset();

        deferredRenderPipeline.reset();
        deferredRenderPipelineLayout.reset();
    }
};

START_SAMPLE(Deferred);

