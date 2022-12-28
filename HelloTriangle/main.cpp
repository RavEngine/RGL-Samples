#include <RGL/RGL.hpp>
#include "Common/App.hpp"
#include <RGL/Device.hpp>
#include <RGL/Surface.hpp>
#include <RGL/Pipeline.hpp>
#include <iostream>
#include <SDL.h>
#include <SDL_syswm.h>

struct HelloWorld : public AppBase {
	std::shared_ptr<RGL::IDevice> device;
	std::shared_ptr<RGL::ISurface> surface;
	std::shared_ptr<RGL::ISwapchain> swapchain;
	std::shared_ptr<RGL::IRenderPass> renderPass;

	const char* SampleName() {
		return "HelloTriangle";
	}
	void init(int argc, char** argv) final {
		RGL::InitOptions options{
			.api = RGL::API::Vulkan,
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

		// create a surface to render to
		// this requires some platform-specific code
		SDL_SysWMinfo wmi;
		SDL_VERSION(&wmi.version);
		if (!SDL_GetWindowWMInfo(window, &wmi)) {
			throw std::runtime_error("Cannot get native window information");
		}
		surface = RGL::CreateSurfaceFromPlatformHandle(
#ifdef _WIN32
			&wmi.info.win.window,
#else
			wmi.info.cocoa.window,
#endif
			true
		);

		int width, height;
		SDL_GetWindowSizeInPixels(window, &width, &height);
		
		// create a swapchain for the surface
		swapchain = device->CreateSwapchain(surface,width,height);

		RGL::RenderPassConfig config{
			.attachments = {
				{
					.format = RGL::TextureFormat::BGRA8_Unorm,
					.loadOp = RGL::LoadAccessOperation::Clear,
					.storeOp = RGL::StoreAccessOperation::Store,
				}
			},
			.subpasses = {
				{
					.type = decltype(config)::SubpassDesc::Type::Graphics,
					.colorAttachmentIndices = {0},
				}
			}
		};

		renderPass = device->CreateRenderPass(config);
	}
	void tick() final {

	}

	void sizechanged(int width, int height) final {
		swapchain->Resize(width, height);
	}

	void shutdown() final {
		// need to null these out before shutting down, otherwise validation errors will occur
		// take care the order that these were initialized in - in general they should be uninitialized in reverse order
		// to ensure all references are cleaned up
		
		renderPass.reset();
		swapchain.reset();
		surface.reset();
		device.reset();

		RGL::Shutdown();
	}
};

START_SAMPLE(HelloWorld);
