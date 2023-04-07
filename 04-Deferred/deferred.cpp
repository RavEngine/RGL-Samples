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

static glm::vec3 cubePositions[]{
    {0,0,0},
    {3,0,0},
    {-3,0,0}
};

struct Deferred : public ExampleFramework {
    RGLPipelineLayoutPtr deferredRenderPipelineLayout, lightRenderPipelineLayout, finalRenderPipelineLayout;
    RGLRenderPipelinePtr deferredRenderPipeline, dirLightRenderPipeline, finalRenderPipeline, selectionRenderPipeline;
    RGLBufferPtr vertexBuffer, indexBuffer, screenTriVerts, imageDownloadBuffer;
        
    RGLCommandBufferPtr commandBuffer;
    RGLTexturePtr depthTexture, colorTexture, normalTexture, positionTexture, lightingTexture, idTexture;
    RGLSamplerPtr textureSampler;
    RGLRenderPassPtr deferredRenderPass, dirLightRenderPass, finalRenderPass;

    int32_t selectedObject = -1;	// default unselected
    
    constexpr static RGL::TextureFormat
        posTexFormat = RGL::TextureFormat::RGBA32_Sfloat,
        normalTexFormat = RGL::TextureFormat::RGBA16_Sfloat,
        colorTexFormat = RGL::TextureFormat::RGBA16_Snorm,
        idTexFormat = RGL::TextureFormat::R32_Uint;
    
    struct alignas(16) {
        glm::mat4 viewProj;
        glm::vec3 pos;
        float timeSinceStart;
        uint32_t objectID;
        
    } deferredStageUbo;

    struct alignas(16) {
        glm::ivec2 scdim;
    } lightingAndFinalStageUbo;
    
    const char* SampleName() {
        return "Deferred";
    }
    void updateGBuffers()
    {
        swapchainFence->Wait();
        // create the depth buffer
        depthTexture = device->CreateTexture({
            .usage = RGL::TextureUsage::DepthStencilAttachment,
            .aspect = RGL::TextureAspect::HasDepth,
            .width = (uint32_t)width,
            .height = (uint32_t)height,
            .format = RGL::TextureFormat::D32SFloat,
            .debugName = "Depth Texture"
            }
        );
        colorTexture = device->CreateTexture({
            .usage = RGL::TextureUsage::ColorAttachment | RGL::TextureUsage::Sampled,
            .aspect = RGL::TextureAspect::HasColor,
            .width = (uint32_t)width,
            .height = (uint32_t)height,
            .format = colorTexFormat,
            .initialLayout = RGL::ResourceLayout::Undefined,
            .debugName = "Color gbuffer"
            }
        );
        normalTexture = device->CreateTexture({
            .usage = RGL::TextureUsage::ColorAttachment | RGL::TextureUsage::Sampled,
            .aspect = RGL::TextureAspect::HasColor,
            .width = (uint32_t)width,
            .height = (uint32_t)height,
            .format = normalTexFormat,
            .initialLayout = RGL::ResourceLayout::Undefined,
            .debugName = "Normal gbuffer"
            }
        );
        positionTexture = device->CreateTexture({
            .usage = RGL::TextureUsage::ColorAttachment | RGL::TextureUsage::Sampled,
            .aspect = RGL::TextureAspect::HasColor,
            .width = (uint32_t)width,
            .height = (uint32_t)height,
            .format = posTexFormat,
            .initialLayout = RGL::ResourceLayout::Undefined,
            .debugName = "Position gbuffer"
            }
        );
        lightingTexture = device->CreateTexture({
            .usage = RGL::TextureUsage::ColorAttachment | RGL::TextureUsage::Sampled,
            .aspect = RGL::TextureAspect::HasColor,
            .width = (uint32_t)width,
            .height = (uint32_t)height,
            .format = colorTexFormat,
            .initialLayout = RGL::ResourceLayout::Undefined,
            .debugName = "Lighting texture"
            }
        );

        idTexture = device->CreateTexture({
            .usage = RGL::TextureUsage::ColorAttachment | RGL::TextureUsage::Sampled | RGL::TextureUsage::TransferSource,
            .aspect = RGL::TextureAspect::HasColor,
            .width = (uint32_t)width,
            .height = (uint32_t)height,
            .format = idTexFormat,
            .initialLayout = RGL::ResourceLayout::Undefined,
            .debugName = "ID texture",
            .readbackEnabled = true
        });

        // the textures begin in the wrong layout
        // we need to transition them to the layout the first pass is expecting
        auto tmpcmd = commandQueue->CreateCommandBuffer();
        auto tmpfence = device->CreateFence(false);
        tmpcmd->Begin();
        for (const auto& ptr : { colorTexture , normalTexture, positionTexture, lightingTexture, idTexture }) {
            tmpcmd->TransitionResource(ptr.get(), RGL::ResourceLayout::Undefined, RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::TransitionPosition::Top);
        }

        tmpcmd->End();
        tmpcmd->Commit({
            .signalFence = tmpfence
        });
        tmpfence->Wait();
    }
    void sampleinit(int argc, char** argv) final {
        
        auto deferredVertexShader = GetShader("deferred.vert");
        auto deferredFragmentShader = GetShader("deferred.frag");
        
        auto screenTriVertexShader = GetShader("screentri.vert");
        auto dirLightFragmentShader = GetShader("dirlight.frag");
        auto finalFragmentShader = GetShader("final.frag");
        auto wireframeFragmentShader = GetShader("selection.frag");
        
        vertexBuffer = device->CreateBuffer({
            RGL::BufferConfig::Type::VertexBuffer,
            sizeof(BasicObjects::Cube::Vertex),
            BasicObjects::Cube::vertices,
            RGL::BufferAccess::Shared
        });
        vertexBuffer->SetBufferData(BasicObjects::Cube::vertices);
        
        screenTriVerts = device->CreateBuffer({
            RGL::BufferConfig::Type::VertexBuffer,
            sizeof(BasicObjects::ScreenTriangle::Vertex),
            BasicObjects::ScreenTriangle::vertices,
            RGL::BufferAccess::Shared
        });
        screenTriVerts->SetBufferData(BasicObjects::ScreenTriangle::vertices);
        
        indexBuffer = device->CreateBuffer({
            RGL::BufferConfig::Type::IndexBuffer,
            sizeof(BasicObjects::Cube::indices[0]),
            BasicObjects::Cube::indices,
            RGL::BufferAccess::Shared
        });
        indexBuffer->SetBufferData(BasicObjects::Cube::indices);

        imageDownloadBuffer = device->CreateBuffer({
            RGL::BufferConfig::Type::StorageBuffer,
            sizeof(uint32_t) * 4,
            sizeof(uint32_t),
            RGL::BufferAccess::Shared,
            RGL::BufferFlags::TransferDestination
         });
        imageDownloadBuffer->MapMemory();

        updateGBuffers();

        textureSampler = device->CreateSampler({});

        // create a pipeline layout
        // this describes what we *can* bind to the shader
        deferredRenderPipelineLayout = device->CreatePipelineLayout({
            .constants = {{ deferredStageUbo, 0, RGL::StageVisibility::Vertex}}
        });
        
        finalRenderPipelineLayout = device->CreatePipelineLayout({
            .bindings = {
                {
                    .binding = 0,
                    .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::CombinedImageSampler,
                    .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
                },
                {
                    .binding = 1,
                    .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::SampledImage,
                    .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
                },
            },
            .boundSamplers = {
                textureSampler
            },
            .constants = {
                {lightingAndFinalStageUbo, 0, RGL::StageVisibility::Fragment}
            }
        });

        lightRenderPipelineLayout = device->CreatePipelineLayout({
            .bindings = {
                 {
                    .binding = 0,
                    .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::CombinedImageSampler,
                    .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
                },
                 {
                    .binding = 1,
                    .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::CombinedImageSampler,
                    .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
                },
                 {
                    .binding = 2,
                    .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::CombinedImageSampler,
                    .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
                },
                 {
                    .binding = 3,
                    .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::CombinedImageSampler,
                    .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
                },
                {
                    .binding = 4,
                    .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::SampledImage,
                    .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
                },       
                {
                    .binding = 5,
                    .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::SampledImage,
                    .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
                },        
                {
                    .binding = 6,
                    .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::SampledImage,
                    .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
                },
                {
                    .binding = 7,
                    .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::SampledImage,
                    .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
                },
            },
            .boundSamplers = {
                textureSampler,
                textureSampler,
                textureSampler,
                textureSampler,
            },
            .constants = {
                {lightingAndFinalStageUbo, 0, RGL::StageVisibility::Fragment}
            }
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
                        },
                        {
                            .format = idTexFormat
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
                .depthStencilConfig = {
                    .depthFormat = RGL::TextureFormat::D32SFloat,
                    .depthTestEnabled = true,
                    .depthWriteEnabled = false,
                    .depthFunction = RGL::DepthCompareFunction::Greater,
                },
                .pipelineLayout = finalRenderPipelineLayout,
        });
        // permutations for the wireframe render
        selectionRenderPipeline = device->CreateRenderPipeline({
            .stages = {
                {
                    .type = RGL::ShaderStageDesc::Type::Vertex,
                    .shaderModule = deferredVertexShader,
                },
                {
                    .type = RGL::ShaderStageDesc::Type::Fragment,
                    .shaderModule = wireframeFragmentShader,
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
                    .polygonOverride = RGL::PolygonOverride::Line,
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
                    .depthTestEnabled = false,
                    .depthWriteEnabled = false,
                    .depthFunction = RGL::DepthCompareFunction::Greater,
                },
                .pipelineLayout = deferredRenderPipelineLayout,
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
                .depthStencilConfig = {
                    .depthFormat = RGL::TextureFormat::D32SFloat,
                    .depthTestEnabled = true,
                    .depthWriteEnabled = false,
                    .depthFunction = RGL::DepthCompareFunction::Greater,
                 },
                .pipelineLayout = lightRenderPipelineLayout,
        });
        
        deferredRenderPass = RGL::CreateRenderPass({
            .attachments = {
                {
                    .format = colorTexFormat,
                    .loadOp = RGL::LoadAccessOperation::Clear,
                    .storeOp = RGL::StoreAccessOperation::Store,
                },
                {
                    .format = normalTexFormat,
                    .loadOp = RGL::LoadAccessOperation::Clear,
                    .storeOp = RGL::StoreAccessOperation::Store,
                },
                {
                    .format = posTexFormat,
                    .loadOp = RGL::LoadAccessOperation::Clear,
                    .storeOp = RGL::StoreAccessOperation::Store,
                }, 
                {
                    .format = idTexFormat,
                    .loadOp = RGL::LoadAccessOperation::Clear,
                    .storeOp = RGL::StoreAccessOperation::Store,
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
                }
            },
            .depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
                .format = RGL::TextureFormat::D32SFloat,
                .loadOp = RGL::LoadAccessOperation::Load,
                .storeOp = RGL::StoreAccessOperation::Store,
            }
        });
        
        finalRenderPass = RGL::CreateRenderPass({
            .attachments = {
                {
                    .format = RGL::TextureFormat::BGRA8_Unorm,
                    .loadOp = RGL::LoadAccessOperation::Clear,
                    .storeOp = RGL::StoreAccessOperation::Store,
                    .clearColor = { 0.4f, 0.6f, 0.9f, 1.0f},
                },
            },
             .depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
                .format = RGL::TextureFormat::D32SFloat,
                .loadOp = RGL::LoadAccessOperation::Load,
                .storeOp = RGL::StoreAccessOperation::Store,
            }
        });

        // the depth texture is not swapchained so we can set it once
        deferredRenderPass->SetDepthAttachmentTexture(depthTexture.get());
        deferredRenderPass->SetAttachmentTexture(0, colorTexture.get());
        deferredRenderPass->SetAttachmentTexture(1, normalTexture.get());
        deferredRenderPass->SetAttachmentTexture(2, positionTexture.get());
        deferredRenderPass->SetAttachmentTexture(3, idTexture.get());

        dirLightRenderPass->SetAttachmentTexture(0, lightingTexture.get());
        dirLightRenderPass->SetDepthAttachmentTexture(depthTexture.get());
        finalRenderPass->SetDepthAttachmentTexture(depthTexture.get());

        // create command buffer
        commandBuffer = commandQueue->CreateCommandBuffer();
        
        camera.position.z = 10;
    }
    void tick() final {
        deferredStageUbo.viewProj = camera.GenerateViewProjMatrix(width, height);
        deferredStageUbo.timeSinceStart = getTimeSeconds();
        lightingAndFinalStageUbo.scdim = { width,height };
        
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
        for (const auto& ptr : { colorTexture, normalTexture, positionTexture, idTexture }) {
            commandBuffer->TransitionResource(ptr.get(), RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::TransitionPosition::Top);
        }

        commandBuffer->BeginRendering(deferredRenderPass);
        commandBuffer->BindPipeline(deferredRenderPipeline);
        commandBuffer->SetViewport({
            .width = static_cast<float>(nextImgSize.width),
            .height = static_cast<float>(nextImgSize.height),
        });
        commandBuffer->SetScissor({
            .extent = {nextImgSize.width, nextImgSize.height}
        });

        commandBuffer->SetVertexBuffer(vertexBuffer);
        commandBuffer->SetIndexBuffer(indexBuffer);
        for (uint32_t i = 0; i < std::size(cubePositions); i++) {
            deferredStageUbo.pos = cubePositions[i];
            deferredStageUbo.objectID = i + 1;      // id 0 is reserved for deselected
            commandBuffer->SetVertexBytes(deferredStageUbo, 0);
            commandBuffer->DrawIndexed(std::size(BasicObjects::Cube::indices));
        }

        commandBuffer->EndRendering();
        
        // then do the lighting pass
        for (const auto& ptr : { colorTexture, normalTexture, positionTexture, idTexture }) {
            commandBuffer->TransitionResource(ptr.get(), RGL::ResourceLayout::ColorAttachmentOptimal, RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::TransitionPosition::Top);
        }
        commandBuffer->TransitionResource(lightingTexture.get(), RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::TransitionPosition::Top);

        commandBuffer->BeginRendering(dirLightRenderPass);
        commandBuffer->BindPipeline(dirLightRenderPipeline);
        commandBuffer->SetViewport({
            .width = static_cast<float>(nextImgSize.width),
            .height = static_cast<float>(nextImgSize.height),
        });
        commandBuffer->SetScissor({
            .extent = {nextImgSize.width, nextImgSize.height}
        });
        commandBuffer->SetFragmentBytes(lightingAndFinalStageUbo, 0);
        commandBuffer->SetCombinedTextureSampler(textureSampler, colorTexture.get(), 0);
        commandBuffer->SetCombinedTextureSampler(textureSampler, normalTexture.get(), 1);
        commandBuffer->SetCombinedTextureSampler(textureSampler, positionTexture.get(), 2);
        commandBuffer->SetVertexBuffer(screenTriVerts);
        commandBuffer->Draw(std::size(BasicObjects::ScreenTriangle::vertices));
        commandBuffer->EndRendering();
        commandBuffer->TransitionResource(lightingTexture.get(), RGL::ResourceLayout::ColorAttachmentOptimal, RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::TransitionPosition::Bottom);

        // next do the final render
        finalRenderPass->SetAttachmentTexture(0, nextimg);

        commandBuffer->TransitionResource(nextimg, RGL::ResourceLayout::Undefined, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::TransitionPosition::Top);
        commandBuffer->BeginRendering(finalRenderPass);

        commandBuffer->SetViewport({
                .width = static_cast<float>(nextImgSize.width),
                .height = static_cast<float>(nextImgSize.height),
            });
        commandBuffer->SetScissor({
                .extent = {nextImgSize.width, nextImgSize.height}
            });

        commandBuffer->BindPipeline(finalRenderPipeline);
        commandBuffer->SetCombinedTextureSampler(textureSampler, lightingTexture.get(), 0);
        commandBuffer->SetVertexBuffer(screenTriVerts);
        commandBuffer->SetFragmentBytes(lightingAndFinalStageUbo, 0);
        commandBuffer->Draw(std::size(BasicObjects::ScreenTriangle::vertices));

        if (selectedObject > -1) {
            // render selected object a second time in wireframe mode
            commandBuffer->BindPipeline(selectionRenderPipeline);
            commandBuffer->SetVertexBuffer(vertexBuffer);
            commandBuffer->SetIndexBuffer(indexBuffer);
            deferredStageUbo.pos = cubePositions[selectedObject - 1];
            commandBuffer->SetVertexBytes(deferredStageUbo, 0);
            commandBuffer->DrawIndexed(std::size(BasicObjects::Cube::indices));
        }
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
        updateGBuffers();
        deferredRenderPass->SetDepthAttachmentTexture(depthTexture.get());    // we recreated it so we need to reset it
        deferredRenderPass->SetAttachmentTexture(0, colorTexture.get());
        deferredRenderPass->SetAttachmentTexture(1, normalTexture.get());
        deferredRenderPass->SetAttachmentTexture(2, positionTexture.get());
        deferredRenderPass->SetAttachmentTexture(3, idTexture.get());
        
        dirLightRenderPass->SetAttachmentTexture(0, lightingTexture.get());
    }

    void updateSelectedObject() {
        int x, y;
        SDL_GetMouseState(&x, &y);
#if __APPLE__
        x *= wmScaleFactor;
        y *= wmScaleFactor;
#endif

        auto tmpcmd = commandQueue->CreateCommandBuffer();
        auto tmpfence = device->CreateFence(false);
        tmpcmd->Begin();
       
        tmpcmd->TransitionResource(idTexture.get(), RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::ResourceLayout::TransferSourceOptimal, RGL::TransitionPosition::Top);
        tmpcmd->CopyTextureToBuffer(idTexture.get(), { .offset = {x,y}, .extent = {1,1} }, 0, imageDownloadBuffer);
        tmpcmd->TransitionResource(idTexture.get(), RGL::ResourceLayout::TransferSourceOptimal, RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::TransitionPosition::Bottom);

        tmpcmd->End();
        tmpcmd->Commit({
            .signalFence = tmpfence
            });
        tmpfence->Wait();

        // now we have the pixel data, so we can read it
        auto dataptr = imageDownloadBuffer->GetMappedDataPtr();
        auto value = *static_cast<uint32_t*>(dataptr);
        if (value == 0) {
            selectedObject = -1;
        }
        else {
            selectedObject = value;
        }
    }

    void sampleevent(SDL_Event& event) final {
        switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
            updateSelectedObject();
        }
    }

    void sampleshutdown() final {

        selectionRenderPipeline.reset();
        finalRenderPass.reset();
        finalRenderPipelineLayout.reset();
        lightRenderPipelineLayout.reset();
        dirLightRenderPipeline.reset();
        dirLightRenderPass.reset();
        finalRenderPipeline.reset();
        screenTriVerts.reset();
        deferredRenderPass.reset();
        depthTexture.reset();
        colorTexture.reset();
        positionTexture.reset();
        imageDownloadBuffer.reset();
        idTexture.reset();
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

