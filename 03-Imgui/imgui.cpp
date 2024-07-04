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

struct ImGuiDemo : public ExampleFramework {
    RGLRenderPipelinePtr renderPipeline;
    RGLBufferPtr vertexBuffer, indexBuffer, instanceDataBuffer;
        
    RGLCommandBufferPtr commandBuffer;
	RGLTexturePtr fontsTexture, depthTexture;
    RGLSamplerPtr textureSampler;
    RGLRenderPassPtr renderPass;
    
    ImGuiIO* imgui_io;
    
    struct UniformData{
        float projMat[4][4];
    } ubo;

	const char* SampleName() {
		return "ImGui";
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
			.format = RGL::TextureFormat::D32SFloat
			}
		);
	}
	void sampleinit(int argc, char** argv) final {
        
        constexpr static const char* const vertShader = R"(
layout(push_constant) uniform UniformBufferObject{
    mat4 projMat;
} ubo;

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in uint inColor;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;

void main(){

    gl_Position = ubo.projMat * vec4(inPos, 0, 1);
    outUV = inUV;
    outColor = unpackUnorm4x8(inColor);
}
)";
        
        constexpr static const char* const fragShader = R"(
layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform texture2D tex;

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;
void main(){

	vec4 texColor = texture(sampler2D(tex,g_sampler), inUV);
    outColor = inColor * texColor;
}
)";
        
        auto vertexShaderLibrary = device->CreateShaderLibrarySourceCode(vertShader, {.stage = RGL::ShaderStage::Vertex});
        auto fragmentShaderLibrary = device->CreateShaderLibrarySourceCode(fragShader, {.stage = RGL::ShaderStage::Fragment});
    
		textureSampler = device->CreateSampler({});

		// create a pipeline layout
		// this describes what we *can* bind to the shader
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
				.vertexBindings = {{
					.binding = 0,
					.stride = sizeof(ImDrawVert),
				}},
                .attributeDescs = {
                    {
                        .location = 0,
                        .binding = 0,
                        .offset = offsetof(ImDrawVert,pos),
                        .format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
                    },
                    {
                        .location = 1,
                        .binding = 0,
                        .offset = offsetof(ImDrawVert,uv),
                        .format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
                    },
                    {
                        .location = 2,
                        .binding = 0,
                        .offset = offsetof(ImDrawVert,col),
                        .format = RGL::VertexAttributeFormat::R32_Uint,
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
                .windingOrder = RGL::WindingOrder::Clockwise,
            },
            .colorBlendConfig{
                .attachments = {
                    {
                        .format = RGL::TextureFormat::BGRA8_Unorm,    // specify attachment data
                 
                        .sourceColorBlendFactor = RGL::BlendFactor::SourceAlpha,
                        .destinationColorBlendFactor = RGL::BlendFactor::OneMinusSourceAlpha,
                        .sourceAlphaBlendFactor = RGL::BlendFactor::One,
                        .destinationAlphaBlendFactor = RGL::BlendFactor::OneMinusSourceAlpha,

                        .colorBlendOperation = RGL::BlendOperation::Add,
                        .alphaBlendOperation = RGL::BlendOperation::Add,

                        .blendEnabled = true
                    }
                }
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
			.usage = {.TransferDestination = true, .Sampled = true},
			.aspect = {.HasColor = true},
			.width = static_cast<uint32_t>(width),
			.height = static_cast<uint32_t>(height),
			.format = RGL::TextureFormat::RGBA8_Unorm,
			.debugName = "ImGui Font Texture"
			},
             {pixels, static_cast<size_t>(width * height * 4)}
		);
		imgui_io->Fonts->SetTexID(fontsTexture.get());
        
        camera.position.z = 5;
	}

	void refreshBuffers(uint32_t vertBufLen, uint32_t indBufLen) {
        if (!vertexBuffer || vertexBuffer->getBufferSize() < vertBufLen){
            vertexBuffer = device->CreateBuffer({
                vertBufLen,
				{.VertexBuffer = true},
                sizeof(ImDrawVert),
				RGL::BufferAccess::Shared
            });
        }
        if (!indexBuffer || indexBuffer->getBufferSize() < indBufLen){
            indexBuffer = device->CreateBuffer({
                vertBufLen,
				{.IndexBuffer = true},
                sizeof(ImDrawIdx),
				RGL::BufferAccess::Shared
            });
        }
	}

	void sampleevent(SDL_Event& event) final{
		ImGui_ImplSDL2_ProcessEvent(&event);
	}

	void tick() final {
		// imgui calls
		ImGui_ImplSDL2_NewFrame();
		ImGui::CreateContext();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();

		ImGui::Render();
		auto drawData = ImGui::GetDrawData();
		size_t vertexBufferLength = (size_t)drawData->TotalVtxCount * sizeof(ImDrawVert);
		size_t indexBufferLength = (size_t)drawData->TotalIdxCount * sizeof(ImDrawIdx);

		int fb_width = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
		int fb_height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
		if (fb_width <= 0 || fb_height <= 0 || drawData->CmdListsCount == 0)
			return;
		
		// now that Imgui has been determined, start the render process
		RGL::SwapchainPresentConfig presentConfig{};
		swapchain->GetNextImage(&presentConfig.imageIndex);
		swapchainFence->Wait();
		swapchainFence->Reset();

		auto nextimg = swapchain->ImageAtIndex(presentConfig.imageIndex);
		auto nextImgSize = nextimg->GetSize();

		renderPass->SetAttachmentTexture(0, nextimg->GetDefaultView());

		// create the unified vert / ind buffer 
		refreshBuffers(vertexBufferLength, indexBufferLength);
        vertexBuffer->MapMemory();
        indexBuffer->MapMemory();

		// Will project scissor/clipping rectangles into framebuffer space
		ImVec2 clip_off = drawData->DisplayPos;         // (0,0) unless using multi-viewports
		ImVec2 clip_scale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
        
        // set the projection matrix
        float L = drawData->DisplayPos.x;
        float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
        float T = drawData->DisplayPos.y;
        float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
        float N = 0;
        float F = 1;
        const float ortho_projection[4][4] =
        {
           { 2.0f/(R-L),   0.0f,           0.0f,   0.0f },
           { 0.0f,         2.0f/(T-B),     0.0f,   0.0f },
           { 0.0f,         0.0f,        1/(F-N),   0.0f },
           { (R+L)/(L-R),  (T+B)/(B-T), N/(F-N),   1.0f },
        };
        memcpy(ubo.projMat, ortho_projection, sizeof(ortho_projection));
        
        commandBuffer->Reset();
        commandBuffer->Begin();

        commandBuffer->BeginRendering(renderPass);
        commandBuffer->BindRenderPipeline(renderPipeline);

		size_t vertexBufferOffset = 0;
		size_t indexBufferOffset = 0;
		for (int n = 0; n < drawData->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = drawData->CmdLists[n];
            
            vertexBuffer->UpdateBufferData({ cmd_list->VtxBuffer.Data, (size_t)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert)}, vertexBufferOffset * sizeof(ImDrawVert));
            indexBuffer->UpdateBufferData({ cmd_list->IdxBuffer.Data, (size_t)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx)}, indexBufferOffset * sizeof(ImDrawIdx));
            
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
						commandBuffer->SetFragmentSampler(textureSampler, 0);
						commandBuffer->SetFragmentTexture(fontsTexture->GetDefaultView(),1);
					}

                    commandBuffer->SetVertexBytes(ubo, 0);
					commandBuffer->SetVertexBuffer(vertexBuffer, { .offsetIntoBuffer = uint32_t(vertexBufferOffset + pcmd->VtxOffset) });
					commandBuffer->SetIndexBuffer(indexBuffer);
					commandBuffer->DrawIndexed(pcmd->ElemCount, {
						.firstIndex = static_cast<uint32_t>(indexBufferOffset + pcmd->IdxOffset)
					});
				}
			}
            vertexBufferOffset += (size_t)cmd_list->VtxBuffer.Size;
            indexBufferOffset += (size_t)cmd_list->IdxBuffer.Size;
		}
        commandBuffer->EndRendering();

		commandBuffer->End();
        vertexBuffer->UnmapMemory();
        indexBuffer->UnmapMemory();
		
		RGL::CommitConfig commitconfig{
			.signalFence = swapchainFence,
		};
		commandBuffer->Commit(commitconfig);
		commandBuffer->BlockUntilCompleted();

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

		vertexBuffer.reset();
		indexBuffer.reset();

		renderPipeline.reset();
	}
};

START_SAMPLE(ImGuiDemo);
