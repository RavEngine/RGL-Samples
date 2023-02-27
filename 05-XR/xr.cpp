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
#include <array>

#define XR_USE_GRAPHICS_API_VULKAN
#include <vulkan/vulkan.h>
#if _WIN32
#define XR_USE_GRAPHICS_API_D3D12
#include <d3d12.h>
#endif

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

struct Cubes : public ExampleFramework {
	float nearZ = 0.1, farZ = 100;
	struct {
		// required for all XR apps
		XrInstance instance;
		XrSession session;
		XrSystemId systemId;
		XrSessionState sessionState = XR_SESSION_STATE_UNKNOWN;

		// place space (usually local or stage)
		XrSpace space;

		// the configs for each display / eye
		std::vector<XrViewConfigurationView> viewConfigurationViews;
		std::vector <XrCompositionLayerProjectionView> projectionViews;
		std::vector<XrView> views;

		union XrSwapchainImage {
			XrSwapchainImageD3D12KHR d3d12Image;
			XrSwapchainImageVulkanKHR vkImage;
		};

		// one list of images per eye
		std::vector<std::vector<XrSwapchainImage>> swapchainImages;
		std::vector<std::vector<XrSwapchainImage>> depthSwapchainImages;
		// one swapchain per eye, plus depth
		int64_t swapchain_format, depth_swapchain_format = -1;
		std::vector<XrSwapchain> swapchains;
		std::vector<XrSwapchain> depth_swapchains;

		// dpeth layer data
		struct
		{
			std::vector<XrCompositionLayerDepthInfoKHR> infos;
			bool supported;
		} depth;

		struct
		{
			std::vector<XrSwapchainImage> images;
			int64_t format;
			uint32_t swapchain_width, swapchain_height;
			uint32_t swapchain_length;
			XrSwapchain swapchain;
			bool supported;
		} cylinder;

		union {
			XrGraphicsBindingD3D12KHR d3d12Binding;
			XrGraphicsBindingVulkan2KHR vkBinding;
		} graphicsBinding;

		XrSessionState state = XR_SESSION_STATE_UNKNOWN;

		XrDebugUtilsMessengerEXT debugMessenger;
		XrViewConfigurationType viewConfigurationType;

		static constexpr XrPosef identity_pose{ 
			.orientation = {.x = 0, .y = 0, .z = 0, .w = 1.0},
			.position = {.x = 0, .y = 0, .z = 0}
		};

	} xr;

	void Fatal(auto str) {
		OutputDebugStringA(str);
		throw std::runtime_error(str);
	}

	void XR_CHECK(XrResult result) {
		if (result != XR_SUCCESS) {
			char resultStr[XR_MAX_RESULT_STRING_SIZE]{0};
			xrResultToString(xr.instance, result, resultStr);
			Fatal(resultStr);
		}
	}


	// xr initialization
	void init_openxr() {

		// get all the extensions
		uint32_t ext_count = 0;
		XR_CHECK(xrEnumerateInstanceExtensionProperties(NULL, 0, &ext_count, NULL));

		std::vector<XrExtensionProperties> extensionProperties(ext_count, { XR_TYPE_EXTENSION_PROPERTIES, nullptr });
		XR_CHECK(xrEnumerateInstanceExtensionProperties(NULL, ext_count, &ext_count, extensionProperties.data()));

		const auto extMatches = [&](uint32_t i, auto name) {
			return std::string_view(extensionProperties[i].extensionName) == name;
		};

		// look for the extension that matches the RGL backend
		bool backendSupported = false;
		const auto currentAPI = RGL::CurrentAPI();
		for (uint32_t i = 0; i < ext_count; i++) {
			if (currentAPI == RGL::API::Direct3D12 && extMatches(i, XR_KHR_D3D12_ENABLE_EXTENSION_NAME)) {
				backendSupported = true;
			}
			else if (currentAPI == RGL::API::Vulkan && extMatches(i, XR_KHR_VULKAN_ENABLE_EXTENSION_NAME)) {
				backendSupported = true;
			}
			
			if (extMatches(i, XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME)) {
				xr.depth.supported = true;
			}
		}
		if (!backendSupported) {
			Fatal("OpenXR Runtime does not support the selected API");
		}

		const char* enabled_exts[] = {currentAPI == RGL::API::Direct3D12 ? XR_KHR_D3D12_ENABLE_EXTENSION_NAME : XR_KHR_VULKAN_ENABLE_EXTENSION_NAME};

		// create the Xr instance
		XrInstanceCreateInfo xrCreateInfo{
			.type = XR_TYPE_INSTANCE_CREATE_INFO,
			.next = nullptr,
			.createFlags = 0,
			.applicationInfo = {
				.applicationName = "RGL OpenXR Example",
				.applicationVersion = 1,
				.engineName = "RGL Example Framework",
				.engineVersion = 1,
				.apiVersion = XR_CURRENT_API_VERSION
			},
			.enabledApiLayerCount = 0,
			.enabledApiLayerNames = nullptr,
			.enabledExtensionCount = std::size(enabled_exts),
			.enabledExtensionNames = enabled_exts
		};

		XR_CHECK(xrCreateInstance(&xrCreateInfo, &xr.instance));

		// create the Xr system for a HMD
		XrSystemGetInfo system_get_info{
			.type = XR_TYPE_SYSTEM_GET_INFO,
			.next = nullptr,
			.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY
		};
		XR_CHECK(xrGetSystem(xr.instance, &system_get_info, &xr.systemId));

		// here one would generally check system properties like hand tracking support, but for this example
		// we don't care about that

		// begin view config
		XrViewConfigurationType view_type = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
		uint32_t view_count = 0;
		XR_CHECK(xrEnumerateViewConfigurationViews(xr.instance, xr.systemId, view_type, 0, &view_count, NULL));
		xr.viewConfigurationViews.resize(view_count, { XR_TYPE_VIEW_CONFIGURATION_VIEW, nullptr });
		XR_CHECK(xrEnumerateViewConfigurationViews(xr.instance, xr.systemId, view_type, view_count, &view_count, xr.viewConfigurationViews.data()));

		// check graphics requirements. OpenXR requires that we call this before creating a session.
		auto devicedata = device->GetDeviceData();
		auto queuedata = commandQueue->GetQueueData();
		if (currentAPI == RGL::API::Direct3D12) {
			XrGraphicsRequirementsD3D12KHR d3d12_reqs{
				.type = XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR,
				.next = nullptr
			};
			PFN_xrGetD3D12GraphicsRequirementsKHR pfn_xrGetD3D12GraphicsRequirementsKHR = nullptr;
			XR_CHECK(xrGetInstanceProcAddr(xr.instance, "xrGetD3D12GraphicsRequirementsKHR", (PFN_xrVoidFunction*)&pfn_xrGetD3D12GraphicsRequirementsKHR));
			XR_CHECK(pfn_xrGetD3D12GraphicsRequirementsKHR(xr.instance, xr.systemId, &d3d12_reqs));


			xr.graphicsBinding.d3d12Binding = XrGraphicsBindingD3D12KHR{
				.type = XR_TYPE_GRAPHICS_BINDING_D3D12_KHR,
				.device = devicedata.d3d12Data.device,
				.queue = queuedata.d3d12Data.commandQueue
			};
		}
		else if (currentAPI == RGL::API::Vulkan) {
			XrGraphicsRequirementsVulkanKHR vk_reqs{
				.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR,
				.next = nullptr
			};
			PFN_xrGetVulkanGraphicsRequirementsKHR pfn_xrGetVulkanGraphicsRequirementsKHR = nullptr;
			XR_CHECK(xrGetInstanceProcAddr(xr.instance, "PFN_xrGetVulkanGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&pfn_xrGetVulkanGraphicsRequirementsKHR));
			XR_CHECK(pfn_xrGetVulkanGraphicsRequirementsKHR(xr.instance, xr.systemId, &vk_reqs));
			xr.graphicsBinding.vkBinding = XrGraphicsBindingVulkan2KHR{
				.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR,
				.device = *(VkDevice*)(devicedata.vkData.device)
			};
		}
		
		// next create the session
		XrGraphicsBindingD3D12KHR graphicsBinding{};
		XrSessionCreateInfo sessionCreateInfo{
			.type = XR_TYPE_SESSION_CREATE_INFO,
			.next = &xr.graphicsBinding,
			.systemId = xr.systemId
		};
		XR_CHECK(xrCreateSession(xr.instance, &sessionCreateInfo, &xr.session));

		// create the reference space
		XrReferenceSpaceType play_space_type = XR_REFERENCE_SPACE_TYPE_LOCAL;
		XrReferenceSpaceCreateInfo playspaceCreateInfo{
			.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO,
			.next = nullptr,
			.referenceSpaceType = play_space_type,
			.poseInReferenceSpace = xr.identity_pose
		};
		XR_CHECK(xrCreateReferenceSpace(xr.session, &playspaceCreateInfo, &xr.space));

		// begin the session
		XrSessionBeginInfo sesionBeginInfo{
			.type = XR_TYPE_SESSION_BEGIN_INFO,
			.next = nullptr,
			.primaryViewConfigurationType = view_type
		};
		XR_CHECK(xrBeginSession(xr.session, &sesionBeginInfo));

		// create swapchains
		uint32_t swapchain_format_count;
		XR_CHECK(xrEnumerateSwapchainFormats(xr.session, 0, &swapchain_format_count, NULL));

		std::vector<int64_t> swapchain_formats(swapchain_format_count);
		XR_CHECK(xrEnumerateSwapchainFormats(xr.session, swapchain_format_count, &swapchain_format_count, swapchain_formats.data()));

			// we will use SRGB for now
		const int preferred_format = currentAPI == RGL::API::Direct3D12 ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
		const int preferred_depth_format = currentAPI == RGL::API::Direct3D12 ? DXGI_FORMAT_D32_FLOAT : VK_FORMAT_D32_SFLOAT;

		xr.swapchain_format = swapchain_formats[0];
		xr.cylinder.format = swapchain_formats[0];
		xr.depth_swapchain_format = swapchain_formats[0];
		for (auto format : swapchain_formats) {
			if (format == preferred_format) {
				xr.swapchain_format = format;
			}
			if (format == preferred_depth_format) {
				xr.depth_swapchain_format = format;
			}
		}

		xr.swapchains.resize(view_count);
		xr.swapchainImages.resize(view_count);
		for (uint32_t i = 0; i < view_count; i++) {
			XrSwapchainCreateInfo swapchain_create_info{
				.type = XR_TYPE_SWAPCHAIN_CREATE_INFO,
				.next = nullptr,
				.createFlags = 0,
				.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT,
				.format = xr.swapchain_format,
				.sampleCount = xr.viewConfigurationViews[i].recommendedSwapchainSampleCount,
				.width = xr.viewConfigurationViews[i].recommendedImageRectWidth,
				.height = xr.viewConfigurationViews[i].recommendedImageRectHeight,
				.faceCount = 1,
				.arraySize = 1,
				.mipCount = 1,
			};
			XR_CHECK(xrCreateSwapchain(xr.session, &swapchain_create_info, &xr.swapchains[i]));

			uint32_t swapchain_length;
			XR_CHECK(xrEnumerateSwapchainImages(xr.swapchains[i], 0, &swapchain_length, nullptr));

			xr.swapchainImages[i].resize(swapchain_length, { currentAPI == RGL::API::Direct3D12 ? XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR : XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR , nullptr });
			XR_CHECK(xrEnumerateSwapchainImages(xr.swapchains[i], swapchain_length, &swapchain_length, (XrSwapchainImageBaseHeader*)xr.swapchainImages[i].data()));
		}

		if (xr.depth_swapchain_format == -1) {
			Fatal("No supported depth swapchain format");
		}

		// the process is much the same for the depth swapchain
		xr.depth_swapchains.resize(view_count);
		xr.depthSwapchainImages.resize(view_count);
		for (uint32_t i = 0; i < view_count; i++) {
			XrSwapchainCreateInfo swapchain_create_info{
				.type = XR_TYPE_SWAPCHAIN_CREATE_INFO,
				.next = nullptr,
				.createFlags = 0,
				.usageFlags = XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				.format = xr.depth_swapchain_format,
				.sampleCount = xr.viewConfigurationViews[i].recommendedSwapchainSampleCount,
				.width = xr.viewConfigurationViews[i].recommendedImageRectWidth,
				.height = xr.viewConfigurationViews[i].recommendedImageRectHeight,
				.faceCount = 1,
				.arraySize = 1,
				.mipCount = 1,
			};
			XR_CHECK(xrCreateSwapchain(xr.session, &swapchain_create_info, &xr.depth_swapchains[i]));

			uint32_t depth_swapchain_length;
			XR_CHECK(xrEnumerateSwapchainImages(xr.depth_swapchains[i], 0, &depth_swapchain_length, nullptr));

			xr.depthSwapchainImages[i].resize(depth_swapchain_length, { currentAPI == RGL::API::Direct3D12 ? XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR : XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR , nullptr });
			XR_CHECK(xrEnumerateSwapchainImages(xr.depth_swapchains[i], depth_swapchain_length, &depth_swapchain_length, (XrSwapchainImageBaseHeader*)xr.depthSwapchainImages[i].data()));
		}

		// a stereo configuration means two views, but we can handle any number
		xr.views.resize(view_count, {XR_TYPE_VIEW, nullptr});
		xr.projectionViews.resize(view_count);
		for (uint32_t i = 0; i < view_count; i++) {
			xr.projectionViews[i] = {
				.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW,
				.next = nullptr,
				.subImage = {
					.swapchain = xr.swapchains[i],
					.imageRect = {
						.offset = {
							.x = 0,
							.y = 0
						},
						.extent = {
							.width = static_cast<int>(xr.viewConfigurationViews[i].recommendedImageRectWidth),
							.height = static_cast<int>(xr.viewConfigurationViews[i].recommendedImageRectHeight)
						}
					},
					.imageArrayIndex = 0,
					// we will fill pose and fov each frame
				}
			};
		}
		xr.depth.infos.resize(view_count);
		for (uint32_t i = 0; i < view_count; i++) {
			xr.depth.infos[i] = {
				.type = XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR,
				.next = nullptr,
				.subImage = {
					.swapchain = xr.depth_swapchains[i],
					.imageRect = {
						.offset = {
							.x = 0,
							.y = 0
						},
						.extent = {
							.width = static_cast<int>(xr.viewConfigurationViews[i].recommendedImageRectWidth),
							.height = static_cast<int>(xr.viewConfigurationViews[i].recommendedImageRectHeight)
						}
					},
					.imageArrayIndex = 0
				},
				.minDepth = 0,
				.maxDepth = 1,
				.nearZ = this->nearZ,
				.farZ = this->farZ,
			};
			xr.projectionViews[i].next = &xr.depth.infos[i];
		}

	}


	RGLRenderPipelinePtr renderPipeline;
	RGLBufferPtr vertexBuffer, indexBuffer;

	RGLCommandBufferPtr commandBuffer;
	RGLTexturePtr depthTexture;
	RGLRenderPassPtr renderPass;

	struct alignas(16) UniformBufferObject {
		glm::mat4 viewProj;
		float timeSinceStart;

	} ubo;

	const char* SampleName() {
		return "OpenXR";
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
		init_openxr();


		auto vertexShaderLibrary = GetShader("xr.vert");
		auto fragmentShaderLibrary = GetShader("xr.frag");

		vertexBuffer = device->CreateBuffer({
			RGL::BufferConfig::Type::VertexBuffer,
			sizeof(BasicObjects::Cube::Vertex),
			BasicObjects::Cube::vertices,
			});
		vertexBuffer->SetBufferData(BasicObjects::Cube::vertices);

		indexBuffer = device->CreateBuffer({
			RGL::BufferConfig::Type::IndexBuffer,
			sizeof(BasicObjects::Cube::indices[0]),
			BasicObjects::Cube::indices,
			});
		indexBuffer->SetBufferData(BasicObjects::Cube::indices);

		createDepthTexture();

		// create a pipeline layout
		// this describes what we *can* bind to the shader
		RGL::PipelineLayoutDescriptor layoutConfig{
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
					.shouldTransition = true		// for swapchain images
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
		commandBuffer->SetVertexBuffer(vertexBuffer);
		commandBuffer->SetIndexBuffer(indexBuffer);
		commandBuffer->DrawIndexed(std::size(BasicObjects::Cube::indices));

		commandBuffer->EndRendering();
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

		renderPass.reset();
		depthTexture.reset();
		commandBuffer.reset();
		commandQueue.reset();

		vertexBuffer.reset();
		indexBuffer.reset();

		renderPipeline.reset();
	}
};

START_SAMPLE(Cubes);
