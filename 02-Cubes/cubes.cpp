#include "Common/ExampleFramework.hpp"
#include <RGL/Pipeline.hpp>
#include <RGL/Buffer.hpp>
#include <RGL/CommandBuffer.hpp>
#include <iostream>
#include <SDL.h>
#include <SDL_syswm.h>
#include <glm/glm.hpp>
#include <format>
#include <filesystem>
#undef CreateSemaphore

struct Cubes : public ExampleFramework {
    std::shared_ptr<RGL::IPipelineLayout> renderPipelineLayout;
    std::shared_ptr<RGL::IRenderPipeline> renderPipeline;
    std::shared_ptr<RGL::IBuffer> vertexBuffer, indexBuffer;
    
    std::shared_ptr<RGL::IShaderLibrary> vertexShaderLibrary, fragmentShaderLibrary;
    
    std::shared_ptr<RGL::ICommandBuffer> commandBuffer;
    
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
    };
    
    struct UniformBufferObject {
        float time = 0;
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

		// create a pipeline layout
		RGL::PipelineLayoutDescriptor layoutConfig{
			.constants = {{ ubo, 0}}
		};
		renderPipelineLayout = device->CreatePipelineLayout(layoutConfig);


		// create a render pipeline
		RGL::RenderPipelineDescriptor rpd{
			.stages = {
				{
					.type = decltype(rpd)::ShaderStageDesc::Type::Vertex,
					.shaderModule = vertexShaderLibrary,
					.entryPoint = "main"
				},
				{
					.type = decltype(rpd)::ShaderStageDesc::Type::Fragment,
					.shaderModule = fragmentShaderLibrary,
					.entryPoint = "main"
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
						.semantic_name = "TEXCOORD",
						.format = decltype(rpd)::VertexConfig::VertexAttributeDesc::Format::R32G32B32_SignedFloat,
					},
					{
						.location = 1,
						.binding = 0,
						.offset = offsetof(Vertex,normal),
						.semantic_name = "TEXCOORD",
						.format = decltype(rpd)::VertexConfig::VertexAttributeDesc::Format::R32G32B32_SignedFloat,
					},
                    {
                        .location = 2,
                        .binding = 0,
                        .offset = offsetof(Vertex,uv),
                        .semantic_name = "TEXCOORD",
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
			.subpassIndex = 0
		};
		renderPipeline = device->CreateRenderPipeline(rpd);

		// create command buffer
		commandBuffer = commandQueue->CreateCommandBuffer();
	}
	void tick() final {
		ubo.time++;
		
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

		
		commandBuffer->BeginRendering({
			.clearColor = { 0.4f, 0.6f, 0.9f, 1.0f},
			.targetFramebuffer = nextimg
		});

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

	void sizechanged(int width, int height) final {
		commandQueue->WaitUntilCompleted();
		swapchain->Resize(width, height);
	}

	void sampleshutdown() final {

		commandBuffer.reset();
		commandQueue.reset();

		vertexBuffer.reset();

		vertexShaderLibrary.reset();
		fragmentShaderLibrary.reset();

		renderPipeline.reset();
		renderPipelineLayout.reset();
	}
};

START_SAMPLE(Cubes);
