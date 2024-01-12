#include <RGL/RGL.hpp>
#include "Common/App.hpp"
#include <RGL/Device.hpp>
#include <RGL/Surface.hpp>
#include <RGL/Pipeline.hpp>
#include <RGL/Swapchain.hpp>
#include <RGL/Buffer.hpp>
#include <RGL/RenderPass.hpp>
#include <RGL/CommandBuffer.hpp>
#include <iostream>
#if defined __linux__ && !defined(__ANDROID__)
	#define SDL_VIDEO_DRIVER_X11 1		//Without this X11 support doesn't work
	#define SDL_VIDEO_DRIVER_WAYLAND 1
#endif
#include <SDL.h>
#include <SDL_syswm.h>
#include <glm/glm.hpp>
#include <filesystem>
#undef CreateSemaphore

struct HelloWorld : public AppBase {
	RGLDevicePtr device;
	RGLSurfacePtr surface;
	RGLSwapchainPtr swapchain;
	RGLRenderPipelinePtr renderPipeline;
	RGLBufferPtr vertexBuffer;

	RGLCommandQueuePtr commandQueue;
	RGLCommandBufferPtr commandBuffer;
	RGLFencePtr swapchainFence;
    RGLRenderPassPtr renderPass;

	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;
	};

	struct UniformBufferObject {
		float time = 0;
	} ubo;

	constexpr static Vertex vertices[] = {
		{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};

	const char* SampleName() {
		return "HelloTriangle";
	}
	void init(int argc, char** argv) final {
		RGL::InitOptions options{
			.api = RGL::API::PlatformDefault,
			.appName = SampleName(),
			.engineName = "RGLSampleFramework",
			.appVersion = {0,0,0,1},
			.engineVersion = {0,0,0,1}
		};
		RGL::Init(options);

		auto backendstr = RGL::APIToString(RGL::CurrentAPI());
		std::cout << backendstr << std::endl;

		// next make a device
		device = RGL::IDevice::CreateSystemDefaultDevice();
		std::cout << device->GetBrandString() << std::endl;

		// create the main queue
		commandQueue = device->CreateCommandQueue(RGL::QueueType::AllCommands);

		// create a surface to render to
		// this requires some platform-specific code
		SDL_SysWMinfo wmi;
		SDL_VERSION(&wmi.version);
		if (!SDL_GetWindowWMInfo(window, &wmi)) {
			#if !__EMSCRIPTEN__
				throw std::runtime_error("Cannot get native window information");
			#endif
		}
		bool isWayland = wmi.subsystem == SDL_SYSWM_WAYLAND;
		surface = RGL::CreateSurfaceFromPlatformHandle(
#if _UWP
			{ wmi.info.winrt.window },
#elif _WIN32
			{ &wmi.info.win.window },
#elif TARGET_OS_IPHONE
			{ wmi.info.uikit.window },
#elif __APPLE__
			{ wmi.info.cocoa.window },
#elif __linux__
			{ isWayland ? static_cast<void*>(wmi.info.wl.display) : static_cast<void*>(wmi.info.x11.display), isWayland ? reinterpret_cast<uintptr_t>(wmi.info.wl.surface) : uintptr_t(wmi.info.x11.window), isWayland },
#elif __EMSCRIPTEN__
            {"#canvas"},
#else
#error Unknown platform
#endif
			true
		);

		int width, height;
#if __EMSCRIPTEN__
        SDL_GetWindowSize(window, &width, &height);
#else
		SDL_GetWindowSizeInPixels(window, &width, &height);
#endif
		
		// create a swapchain for the surface
		// provide it the queue which will be presented on
		swapchain = device->CreateSwapchain(surface, commandQueue, width * wmScaleFactor,height*wmScaleFactor);
		swapchainFence = device->CreateFence(true); 

		auto getShaderPathname = [](const std::string& name) {
			const char* backendPath;
			const char* extension;
			switch (RGL::CurrentAPI()) {
			case RGL::API::Vulkan:
				backendPath = "Vulkan";
				extension = ".spv";
				return std::filesystem::path("../shaders") / backendPath / (name + extension);
				break;
			case RGL::API::Direct3D12:
				backendPath = "Direct3D12";
				extension = ".cso";
				return std::filesystem::path(name + extension);
				break;
			case RGL::API::WebGPU:
				backendPath = "WebGPU";
				extension = ".wgsl";
				return std::filesystem::path("shaders") / "WebGPU" / std::filesystem::path(name + extension);
				break;
			default:
				throw std::runtime_error("Shader loading not implemented");
			}
			
		};

#if __APPLE__
        RGLShaderLibraryPtr vertexShaderLibrary = device->CreateShaderLibraryFromName("triangle_vert");
        RGLShaderLibraryPtr fragmentShaderLibrary = device->CreateShaderLibraryFromName("triangle_frag");
#else
		// load our shaders
        RGLShaderLibraryPtr vertexShaderLibrary = device->CreateShaderLibraryFromPath(getShaderPathname("triangle.vert"));
        RGLShaderLibraryPtr fragmentShaderLibrary = device->CreateShaderLibraryFromPath(getShaderPathname("triangle.frag"));
#endif

		vertexBuffer = device->CreateBuffer({
			{.VertexBuffer = true},
			sizeof(Vertex),
			vertices,
			RGL::BufferAccess::Private
		});
		vertexBuffer->SetBufferData(vertices);

		// create a pipeline layout
		RGL::PipelineLayoutDescriptor layoutConfig{
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
					.stride = sizeof(Vertex),
				}},
                .attributeDescs = {
                    {
                        .location = 0,
                        .binding = 0,
                        .offset = offsetof(Vertex,pos),
                        .format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
                    },
                    {
                        .location = 1,
                        .binding = 0,
                        .offset = offsetof(Vertex,color),
                        .format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
                    }
                }
            },
            .inputAssembly = {
                .topology = RGL::PrimitiveTopology::TriangleList,
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
            .pipelineLayout = renderPipelineLayout,
        });
        
		renderPass = RGL::CreateRenderPass({
			.attachments = {
				{
					.format = RGL::TextureFormat::BGRA8_Unorm,
					.loadOp = RGL::LoadAccessOperation::Clear,
					.storeOp = RGL::StoreAccessOperation::Store,
					.clearColor = { 0.4f, 0.6f, 0.9f, 1.0f}
                }
            }
        });

		// create command buffer
		commandBuffer = commandQueue->CreateCommandBuffer();
	}
	void tick() final {
		ubo.time++;
		
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
		commandBuffer->Draw(std::size(vertices));

		commandBuffer->EndRendering();

		commandBuffer->End();
		
		RGL::CommitConfig commitconfig{
			.signalFence = swapchainFence,
		};
		commandBuffer->Commit(commitconfig);
		commandBuffer->BlockUntilCompleted();

		swapchain->Present(presentConfig);
	}

	void sizechanged(int width, int height) final {
		commandQueue->WaitUntilCompleted();
		swapchain->Resize(width, height);
	}

	void shutdown() final {
		commandQueue->WaitUntilCompleted();
		device->BlockUntilIdle();

		// need to null these out before shutting down, otherwise validation errors will occur
		// take care the order that these were initialized in - in general they should be uninitialized in reverse order
		// to ensure all references are cleaned up
        renderPass.reset();
		commandBuffer.reset();
		commandQueue.reset();

		vertexBuffer.reset();

		renderPipeline.reset();
		swapchainFence.reset();
		swapchain.reset();
		surface.reset();
		device.reset();

		RGL::Shutdown();
	}
};

START_SAMPLE(HelloWorld);
