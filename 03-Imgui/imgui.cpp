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
#include <imgui_impl_sdl.cpp>

#undef CreateSemaphore
#undef LoadImage

struct Cubes : public ExampleFramework {
	RGLPipelineLayoutPtr renderPipelineLayout;
    RGLRenderPipelinePtr renderPipeline;
    RGLBufferPtr vertexBuffer, indexBuffer, instanceDataBuffer;
    
    RGLShaderLibraryPtr vertexShaderLibrary, fragmentShaderLibrary;
    
    RGLCommandBufferPtr commandBuffer;
	RGLTexturePtr fontsTexture, depthTexture;
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
		ImGui_ImplSDL2_Init(window,nullptr);
        imgui_io = &ImGui::GetIO();
		imgui_io->BackendRendererUserData = this;
		imgui_io->BackendRendererName = "imgui_impl_RGL";
		imgui_io->BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
        ImGui::StyleColorsDark();

		// create the font texture
		unsigned char* pixels;
		int width, height;
		imgui_io->Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		fontsTexture = device->CreateTextureWithData(RGL::TextureConfig{
			.usage = (RGL::TextureUsage::Sampled | RGL::TextureUsage::TransferDestination),
			.aspect = RGL::TextureAspect::HasColor,
			.width = static_cast<uint32_t>(width),
			.height = static_cast<uint32_t>(height),
			.format = RGL::TextureFormat::RGBA8_Unorm
			},
			pixels
		);
		imgui_io->Fonts->SetTexID(fontsTexture.get());
        
        camera.position.z = 5;
	}

	void refreshBuffers(uint32_t vertBufLen, uint32_t indBufLen) {
		//TODO: if the new requested size > current buffer size, regen buffer

		vertexBuffer = device->CreateBuffer({
			vertBufLen,
			RGL::BufferConfig::Type::VertexBuffer,
			sizeof(ImDrawVert),
		});
		indexBuffer = device->CreateBuffer({
			vertBufLen,
			RGL::BufferConfig::Type::IndexBuffer,
			sizeof(ImDrawIdx),
		});
		vertexBuffer->MapMemory();
		indexBuffer->MapMemory();
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
		auto drawData = ImGui::GetDrawData();
		size_t vertexBufferLength = (size_t)drawData->TotalVtxCount * sizeof(ImDrawVert);
		size_t indexBufferLength = (size_t)drawData->TotalIdxCount * sizeof(ImDrawIdx);
		
		// create the unified vert / ind buffer 
		refreshBuffers(vertexBufferLength, indexBufferLength);

		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		int fb_width = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
		int fb_height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
		if (fb_width <= 0 || fb_height <= 0 || drawData->CmdListsCount == 0)
			return;

		// Will project scissor/clipping rectangles into framebuffer space
		ImVec2 clip_off = drawData->DisplayPos;         // (0,0) unless using multi-viewports
		ImVec2 clip_scale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		commandBuffer->Begin();
		commandBuffer->BindPipeline(renderPipeline);
		size_t vertexBufferOffset = 0;
		size_t indexBufferOffset = 0;
		for (int n = 0; n < drawData->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = drawData->CmdLists[n];
			// TODO: populate buffers considering offset
			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback)
				{
					// User callback, registered via ImDrawList::AddCallback()
					// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
					if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
						//TODO: nuke all the state 
					}
					else
						pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					// Project scissor/clipping rectangles into framebuffer space
					ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
					ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

					// Clamp to viewport as setScissorRect() won't accept values that are off bounds
					if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
					if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
					if (clip_max.x > fb_width) { clip_max.x = (float)fb_width; }
					if (clip_max.y > fb_height) { clip_max.y = (float)fb_height; }
					if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
						continue;
					if (pcmd->ElemCount == 0) // drawIndexedPrimitives() validation doesn't accept this
						continue;

					commandBuffer->BeginRendering(renderPass);

					commandBuffer->SetViewport({
						.width = static_cast<float>(nextImgSize.width),
						.height = static_cast<float>(nextImgSize.height),
						});
					commandBuffer->SetScissor({
						.offset = {
							static_cast<int>(clip_min.x),
							static_cast<int>(clip_min.y)
						},
						.extent = {
							static_cast<uint32_t>(clip_max.x - clip_min.x),
							static_cast<uint32_t>(clip_max.y - clip_min.y),
						}
						});

					if (ImTextureID tex_id = pcmd->GetTexID()) {
						//TODO: texture vs no texture
					}

					//TODO: set buffer offsets
					commandBuffer->SetIndexBuffer(indexBuffer);
					commandBuffer->BindBuffer(vertexBuffer, 0);
					commandBuffer->DrawIndexed(pcmd->ElemCount, {
						.firstIndex = static_cast<uint32_t>(indexBufferOffset + pcmd->IdxOffset * sizeof(ImDrawIdx))
					});
					commandBuffer->EndRendering();
				}
			}
			vertexBufferOffset += (size_t)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
			indexBufferOffset += (size_t)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
		}

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
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
        renderPass.reset();
		depthTexture.reset();
		fontsTexture.reset();
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
