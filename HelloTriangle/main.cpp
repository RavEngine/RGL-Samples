#include <RGL/RGL.hpp>
#include "Common/App.hpp"
#include <RGL/Device.hpp>
#include <RGL/Surface.hpp>
#include <RGL/Pipeline.hpp>
#include <iostream>
#include <SDL.h>
#include <SDL_syswm.h>
#include <glm/glm.hpp>

struct HelloWorld : public AppBase {
	std::shared_ptr<RGL::IDevice> device;
	std::shared_ptr<RGL::ISurface> surface;
	std::shared_ptr<RGL::ISwapchain> swapchain;
	std::shared_ptr<RGL::IRenderPass> renderPass;
	std::shared_ptr<RGL::IPipelineLayout> renderPipelineLayout;
	std::shared_ptr<RGL::IRenderPipeline> renderPipeline;

	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;
	};

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

		// create a renderpass
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

		// create a pipeline layout
		RGL::PipelineLayoutDescriptor layoutConfig{

		};
		renderPipelineLayout = device->CreatePipelineLayout(layoutConfig);

		// create a render pipeline
		RGL::RenderPipelineDescriptor rpd{
			.stages = {
				{
					.type = decltype(rpd)::ShaderStageDesc::Type::Vertex,
					.shaderModule = nullptr, // TODO: fix
					.entryPoint = "main"
				},
				{
					.type = decltype(rpd)::ShaderStageDesc::Type::Fragment,
					.shaderModule = nullptr, // TODO: fix
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
						.format = decltype(rpd)::VertexConfig::VertexAttributeDesc::Format::R32G32_SignedFloat,
					},
					{
						.location = 1,
						.binding = 0,
						.offset = offsetof(Vertex,color),
						.format = decltype(rpd)::VertexConfig::VertexAttributeDesc::Format::R32G32B32_SignedFloat,
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
			.colorBlendConfig{
				.attachments = {{}}	// default init one attachment to match the attachment earlier
			},
			.pipelineLayout = nullptr,	//TODO: pipelinelayout
			.renderpass = renderPass,
			.subpassIndex = 0
		};
		renderPipeline = device->CreateRenderPipeline(renderPipelineLayout, renderPass, rpd);
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

		renderPipeline.reset();
		renderPipelineLayout.reset();
		renderPass.reset();
		swapchain.reset();
		surface.reset();
		device.reset();

		RGL::Shutdown();
	}
};

START_SAMPLE(HelloWorld);
