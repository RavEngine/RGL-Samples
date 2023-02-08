#include "Common/ExampleFramework.hpp"
#include <RGL/Pipeline.hpp>
#include <RGL/Buffer.hpp>
#include <RGL/CommandBuffer.hpp>
#include <RGL/Texture.hpp>
#include <RGL/Sampler.hpp>
#include <iostream>
#include <SDL.h>
#include <SDL_syswm.h>
#undef CreateSemaphore
#undef LoadImage

struct Cubes : public ExampleFramework {
	RGLPipelineLayoutPtr renderPipelineLayout;
    RGLRenderPipelinePtr renderPipeline;
    RGLBufferPtr vertexBuffer, indexBuffer;
    
    RGLShaderLibraryPtr vertexShaderLibrary, fragmentShaderLibrary;
    
    std::shared_ptr<RGL::ICommandBuffer> commandBuffer;
	RGLTexturePtr sampledTexture;
    RGLSamplerPtr textureSampler;
    RGLRenderPassPtr renderPass;
    
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
    };
    
    struct UniformBufferObject {
        glm::mat4 viewProj;
        
    } ubo;
    
    constexpr static Vertex vertices[] = {
        {{ -1.0,  1.0,  1.0}, { 0.0,  0.0,  1.0}, {0, 0 }},
        {{ 1.0,  1.0,  1.0}, { 0.0,  0.0,  1.0}, {1, 0 }},
        {{ -1.0, -1.0,  1.0}, { 0.0,  0.0,  1.0}, {0, 1 }},
        {{ 1.0, -1.0,  1.0}, { 0.0,  0.0,  1.0}, {1, 1 }},
        {{ -1.0, 1.0, -1.0}, { 0.0,  0.0, -1.0}, {0, 0 }},
        {{ 1.0,  1.0, -1.0}, { 0.0,  0.0, -1.0}, {1, 0 }},
        {{ -1.0, -1.0, -1.0}, { 0.0,  0.0, -1.0}, {0, 1 }},
        {{ 1.0, -1.0, -1.0}, { 0.0,  0.0, -1.0}, {1, 1 }},
        {{ -1.0, 1.0,  1.0}, { 0.0,  1.0,  0.0}, {0, 0 }},
        {{ 1.0,  1.0,  1.0}, { 0.0,  1.0,  0.0}, {1, 0 }},
        {{ -1.0, 1.0, -1.0}, { 0.0,  1.0,  0.0}, {0, 1 }},
        {{ 1.0,  1.0, -1.0}, { 0.0,  1.0,  0.0}, {1, 1 }},
        {{ -1.0, -1.0,  1.0}, { 0.0, -1.0,  0.0}, {0, 0 }},
        {{ 1.0, -1.0,  1.0}, { 0.0, -1.0,  0.0}, {1, 0 }},
        {{ -1.0, -1.0, -1.0}, { 0.0, -1.0,  0.0}, {0, 1 }},
        {{ 1.0, -1.0, -1.0}, { 0.0, -1.0,  0.0}, {1, 1 }},
        {{ 1.0, -1.0,  1.0}, { 1.0,  0.0,  0.0}, {0, 0 }},
        {{ 1.0,  1.0,  1.0}, { 1.0,  0.0,  0.0}, {1, 0 }},
        {{ 1.0, -1.0, -1.0}, { 1.0,  0.0,  0.0}, {0, 1 }},
        {{ 1.0,  1.0, -1.0}, { 1.0,  0.0,  0.0}, {1, 1 }},
        {{ -1.0, -1.0,  1.0}, { -1.0,  0.0,  0.0}, {0, 0 }},
        {{ -1.0, 1.0,  1.0}, { -1.0,  0.0,  0.0}, {1, 0 }},
        {{ -1.0, -1.0, -1.0}, { -1.0,  0.0,  0.0}, {0, 1 }},
        {{ -1.0, 1.0, -1.0}, { -1.0,  0.0,  0.0}, {1, 1 }},
	};
    constexpr static uint32_t indices[] = {
        0,  2,  1,
        1,  2,  3,
        4,  5,  6,
        5,  7,  6,

        8, 10,  9,
        9, 10, 11,
       12, 13, 14,
       13, 15, 14,

       16, 18, 17,
       17, 18, 19,
       20, 21, 22,
       21, 23, 22,
    };

	const char* SampleName() {
		return "Cubes";
	}
	void sampleinit(int argc, char** argv) final {
        
        vertexShaderLibrary = GetShader("cubes.vert");
        fragmentShaderLibrary = GetShader("cubes.frag");
        
		vertexBuffer = device->CreateBuffer({
			RGL::BufferConfig::Type::VertexBuffer,
			sizeof(Vertex),
			vertices,
		});
		vertexBuffer->SetBufferData(vertices);
        
        indexBuffer = device->CreateBuffer({
            RGL::BufferConfig::Type::IndexBuffer,
            sizeof(indices[0]),
            indices,
        });
        indexBuffer->SetBufferData(indices);

		auto imagedata = LoadImage("tx1.png");

		sampledTexture = device->CreateTextureWithData({ .width = imagedata.width, .height = imagedata.height, .format = RGL::TextureFormat::RGBA8_SFloat }, imagedata.bytes);

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
			},
			.boundSamplers = {
				textureSampler
			},
			.constants = {{ ubo, 0}}
		};
		renderPipelineLayout = device->CreatePipelineLayout(layoutConfig);


		// then give the layout the data for it to represent
		
		renderPipelineLayout->SetLayout({
			.boundTextures = {
				{
					.texture = sampledTexture,
					.sampler = textureSampler,
				}
			}
		});


		// create a render pipeline
		RGL::RenderPipelineDescriptor rpd{
			.stages = {
				{
					.type = decltype(rpd)::ShaderStageDesc::Type::Vertex,
					.shaderModule = vertexShaderLibrary,
				},
				{
					.type = decltype(rpd)::ShaderStageDesc::Type::Fragment,
					.shaderModule = fragmentShaderLibrary,
				}
			},
			.vertexConfig = {
				.vertexBindinDesc = {
					.binding = 0,
					.stride = sizeof(Vertex),
				},
				.attributeDescs = {
					{
						.location = 0,
						.binding = 0,
						.offset = offsetof(Vertex,pos),
						.format = decltype(rpd)::VertexConfig::VertexAttributeDesc::Format::R32G32B32_SignedFloat,
					},
					{
						.location = 1,
						.binding = 0,
						.offset = offsetof(Vertex,normal),
						.format = decltype(rpd)::VertexConfig::VertexAttributeDesc::Format::R32G32B32_SignedFloat,
					},
                    {
                        .location = 2,
                        .binding = 0,
                        .offset = offsetof(Vertex,uv),
                        .format = decltype(rpd)::VertexConfig::VertexAttributeDesc::Format::R32G32_SignedFloat,
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
				.windingOrder = decltype(rpd)::RasterizerConfig::WindingOrder::Counterclockwise,
			},
			.colorBlendConfig{
				.attachments = {
					{
						.format = RGL::TextureFormat::BGRA8_Unorm	// specify attachment data
					}
				}
			},
			.pipelineLayout = renderPipelineLayout,
		};
		renderPipeline = device->CreateRenderPipeline(rpd);
        
		renderPass = std::make_shared<RGLRenderPassPtr::element_type>(RGL::RenderPassConfig{
			.attachments = {
				{
					.format = RGL::TextureFormat::BGRA8_Unorm,
					.loadOp = RGL::LoadAccessOperation::Clear,
					.storeOp = RGL::StoreAccessOperation::Store,
					.clearColor = { 0.4f, 0.6f, 0.9f, 1.0f},
					.shouldTransition = true		// for swapchain images
                }
            }
        });

		// create command buffer
		commandBuffer = commandQueue->CreateCommandBuffer();
        
        camera.position.z = 5;
	}
	void tick() final {
        ubo.viewProj = camera.GenerateViewProjMatrix(width, height);
		
		RGL::SwapchainPresentConfig presentConfig{
			.waitSemaphores = {&renderCompleteSemaphore,1}
		};

		swapchain->GetNextImage(&presentConfig.imageIndex, imageAvailableSemaphore);
		swapchainFence->Wait();
		swapchainFence->Reset();
		commandBuffer->Reset();
		commandBuffer->Begin();
		auto nextimg = swapchain->ImageAtIndex(presentConfig.imageIndex);
		auto nextImgSize = nextimg->GetSize();

        renderPass->SetAttachmentTexture(0, nextimg);

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

		commandBuffer->BindBuffer(vertexBuffer,0);
        commandBuffer->SetIndexBuffer(indexBuffer);
		commandBuffer->DrawIndexed(std::size(indices));

		commandBuffer->EndRendering();
		commandBuffer->End();
		
		RGL::CommitConfig commitconfig{
			.signalFence = swapchainFence,
			.waitSemaphores = {&imageAvailableSemaphore,1},
			.signalSemaphores = {&renderCompleteSemaphore, 1}
		};
		commandBuffer->Commit(commitconfig);

		swapchain->Present(presentConfig);
	}

	void sampleshutdown() final {

        renderPass.reset();
		sampledTexture.reset();
		textureSampler.reset();
		commandBuffer.reset();
		commandQueue.reset();

		vertexBuffer.reset();
		indexBuffer.reset();

		vertexShaderLibrary.reset();
		fragmentShaderLibrary.reset();

		renderPipeline.reset();
		renderPipelineLayout.reset();
	}
};

START_SAMPLE(Cubes);
