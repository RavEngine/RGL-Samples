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

struct Cubes : public ExampleFramework {
    RGLRenderPipelinePtr renderPipeline;
    RGLBufferPtr vertexBuffer, indexBuffer, instanceDataBuffer;
        
    RGLCommandBufferPtr commandBuffer;
	RGLTexturePtr sampledTexture, depthTexture;
    RGLSamplerPtr textureSampler;
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
        
		vertexBuffer = device->CreateBuffer({
			RGL::BufferConfig::Type::VertexBuffer,
			sizeof(BasicObjects::Cube::Vertex),
            BasicObjects::Cube::vertices,
		});
		vertexBuffer->SetBufferData(BasicObjects::Cube::vertices);
        
        // seed the buffer with random values
        std::random_device rd;
        std::mt19937 mt{rd()};
        std::uniform_real_distribution<> distr(-5, 5);
        for(auto& value : cubeSpinSpeeds){
            value = distr(mt);
        }
        
        instanceDataBuffer = device->CreateBuffer({
           RGL::BufferConfig::Type::StorageBuffer,
            sizeof(decltype(cubeSpinSpeeds[0])),
            cubeSpinSpeeds
        });
        instanceDataBuffer->SetBufferData(cubeSpinSpeeds);
        
        
        indexBuffer = device->CreateBuffer({
            RGL::BufferConfig::Type::IndexBuffer,
            sizeof(BasicObjects::Cube::indices[0]),
            BasicObjects::Cube::indices,
        });
        indexBuffer->SetBufferData(BasicObjects::Cube::indices);

		auto imagedata = LoadImage("tx1.png");

		sampledTexture = device->CreateTextureWithData(RGL::TextureConfig{ 
			.usage = (RGL::TextureUsage::Sampled | RGL::TextureUsage::TransferDestination),
			.aspect = RGL::TextureAspect::HasColor,
			.width = imagedata.width, 
			.height = imagedata.height, 
			.format = RGL::TextureFormat::RGBA8_Unorm
			}, 
			imagedata.bytes
		);

		createDepthTexture();

		textureSampler = device->CreateSampler({});

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
				textureSampler
			},
			.constants = {{ ubo, 0}}
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
		commandBuffer->SetVertexBuffer(vertexBuffer);
        commandBuffer->SetIndexBuffer(indexBuffer);
		commandBuffer->SetCombinedTextureSampler(textureSampler, sampledTexture.get(), 0);
		commandBuffer->DrawIndexed(std::size(BasicObjects::Cube::indices), {
            .nInstances = nCubes
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
		renderPass->SetDepthAttachmentTexture(depthTexture.get());	// we recreated it so we need to reset it
	}

	void sampleshutdown() final {

		instanceDataBuffer.reset();
        renderPass.reset();
		depthTexture.reset();
		sampledTexture.reset();
		textureSampler.reset();
		commandBuffer.reset();
		commandQueue.reset();

		vertexBuffer.reset();
		indexBuffer.reset();

		renderPipeline.reset();
	}
};

START_SAMPLE(Cubes);
