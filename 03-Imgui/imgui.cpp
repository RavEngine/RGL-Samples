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
#include <imgui.h>
#include <imgui_impl_sdl.h>

#undef CreateSemaphore
#undef LoadImage

struct Cubes : public ExampleFramework {
	RGLPipelineLayoutPtr renderPipelineLayout;
    RGLRenderPipelinePtr renderPipeline;
    RGLBufferPtr vertexBuffer, indexBuffer, instanceDataBuffer;
    
    RGLShaderLibraryPtr vertexShaderLibrary, fragmentShaderLibrary;
    
    RGLCommandBufferPtr commandBuffer;
	RGLTexturePtr sampledTexture, depthTexture;
    RGLSamplerPtr textureSampler;
    RGLRenderPassPtr renderPass;
    
    ImGuiIO* imgui_io;
    
    struct Vertex {
        glm::vec3 pos;
    };

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
        
        constexpr static const char* const vertShader = R"(
void main(){

    gl_Position = vec4(1,1,1,1);
}
)";
        
        constexpr static const char* const fragShader = R"(
layout(location = 0) out vec4 outcolor;
void main(){

    outcolor = vec4(1,1,1,1);
}
)";
        
        vertexShaderLibrary = device->CreateShaderLibrarySourceCode(vertShader, {.stage = RGL::ShaderStage::Vertex});
        fragmentShaderLibrary = device->CreateShaderLibrarySourceCode(fragShader, {.stage = RGL::ShaderStage::Fragment});
    
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
			.constants = {}
		};
		renderPipelineLayout = device->CreatePipelineLayout(layoutConfig);


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
        
		renderPass = RGL::CreateRenderPass({
			.attachments = {
				{
					.format = RGL::TextureFormat::BGRA8_Unorm,
					.loadOp = RGL::LoadAccessOperation::Clear,
					.storeOp = RGL::StoreAccessOperation::Store,
					.clearColor = { 0.4f, 0.6f, 0.9f, 1.0f},
					.shouldTransition = true		// for swapchain images
				}
			},
		});

		// create command buffer
		commandBuffer = commandQueue->CreateCommandBuffer();
        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        imgui_io = &ImGui::GetIO();
		
        ImGui::StyleColorsDark();
        
        camera.position.z = 5;
	}

	void onevent(SDL_Event& event) final{
		ImGui_ImplSDL2_ProcessEvent(&event);
	}

	void tick() final {
		
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();
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

		ImGui::Render();

        commandBuffer->BeginRendering(renderPass);

		commandBuffer->SetViewport({
				.width = static_cast<float>(nextImgSize.width),
				.height = static_cast<float>(nextImgSize.height),
			});
		commandBuffer->SetScissor({
				.extent = {nextImgSize.width, nextImgSize.height}
			});

        /*
		commandBuffer->BindPipeline(renderPipeline);
		commandBuffer->SetVertexBytes(ubo, 0);
        commandBuffer->BindBuffer(instanceDataBuffer, 2);
		commandBuffer->BindBuffer(vertexBuffer,0);
        commandBuffer->SetIndexBuffer(indexBuffer);
		commandBuffer->DrawIndexed(std::size(indices), {
            .nInstances = nCubes
		});
         */

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
		depthTexture.reset();
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
