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

#if RGL_VK_AVAILABLE
#define XR_USE_GRAPHICS_API_VULKAN
#include <vulkan/vulkan.h>
#include <RGL/../../src/VkTexture.hpp>
#endif
#if _WIN32
#define XR_USE_GRAPHICS_API_D3D12
#include <RGL/../../src/D3D12Texture.hpp>
#include <d3d12.h>
#endif

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

struct XR : public ExampleFramework {
	struct alignas(16) UniformBufferObject {
		glm::mat4 viewProj;
		float timeSinceStart;

	} ubo;
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
#if RGL_DX12_AVAILABLE
			XrSwapchainImageD3D12KHR d3d12Image;
#endif
#if RGL_VK_AVAILABLE
			XrSwapchainImageVulkanKHR vkImage;
#endif
		};

		// one list of images per eye
		std::vector<std::vector<XrSwapchainImage>> swapchainImages;
		std::vector<std::vector<std::unique_ptr<RGL::ITexture>>> rglSwapchainImages;
		std::vector<std::vector<XrSwapchainImage>> depthSwapchainImages;
		std::vector<std::vector<std::unique_ptr<RGL::ITexture>>> rglDepthSwapchainImages;
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
#if RGL_DX12_AVAILABLE
			XrGraphicsBindingD3D12KHR d3d12Binding;
#endif
#if RGL_VK_AVAILABLE
			XrGraphicsBindingVulkan2KHR vkBinding;
#endif
		} graphicsBinding;

		XrSessionState state = XR_SESSION_STATE_UNKNOWN;

		XrDebugUtilsMessengerEXT debugMessenger;
		XrViewConfigurationType viewConfigurationType;

		inline static PFN_xrCreateDebugUtilsMessengerEXT ext_xrCreateDebugUtilsMessengerEXT = nullptr;
		inline static PFN_xrDestroyDebugUtilsMessengerEXT ext_xrDestroyDebugUtilsMessengerEXT = nullptr;

		static constexpr XrPosef identity_pose{ 
			.orientation = {.x = 0, .y = 0, .z = 0, .w = 1.0},
			.position = {.x = 0, .y = 0, .z = 0}
		};

	} xr;

	void Fatal(auto str) {
		OutputDebugStringA(str);
		SDL_ShowSimpleMessageBox(0, "Fatal Error", str, window);
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
#if RGL_DX12_AVAILABLE
			if (currentAPI == RGL::API::Direct3D12 && extMatches(i, XR_KHR_D3D12_ENABLE_EXTENSION_NAME)) {
				backendSupported = true;
			}
#endif
#if RGL_VK_AVAILABLE
			if (currentAPI == RGL::API::Vulkan && extMatches(i, XR_KHR_VULKAN_ENABLE_EXTENSION_NAME)) {
				backendSupported = true;
			}
#endif
			
			if (extMatches(i, XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME)) {
				xr.depth.supported = true;
			}
		}
		if (!backendSupported) {
			Fatal(std::format("OpenXR Runtime does not support the selected API: {}",RGL::APIToString(currentAPI)).c_str());
		}

		const char* apiStr = nullptr;
		if (currentAPI == RGL::API::Direct3D12) {
#if RGL_DX12_AVAILABLE
			apiStr = XR_KHR_D3D12_ENABLE_EXTENSION_NAME;
#endif
		}
		else {
#if RGL_VK_AVAILABLE
			apiStr = XR_KHR_VULKAN_ENABLE_EXTENSION_NAME;
#endif
		}

		const char* enabled_exts[] = {
			apiStr,
			XR_EXT_DEBUG_UTILS_EXTENSION_NAME
		};

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

		// setup debug messenger
		XR_CHECK(xrGetInstanceProcAddr(xr.instance, "xrCreateDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)(&xr.ext_xrCreateDebugUtilsMessengerEXT)));
		XR_CHECK(xrGetInstanceProcAddr(xr.instance, "xrDestroyDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)(&xr.ext_xrDestroyDebugUtilsMessengerEXT)));

		// create debug log
		XrDebugUtilsMessengerCreateInfoEXT debug_info = { XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };

		debug_info.messageTypes =
			XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
			XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
		debug_info.messageSeverities =
			XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debug_info.userCallback = [](XrDebugUtilsMessageSeverityFlagsEXT severity, XrDebugUtilsMessageTypeFlagsEXT types, const XrDebugUtilsMessengerCallbackDataEXT* msg, void* user_data) {

			OutputDebugStringA(std::format("[OpenXR] {}: {}", msg->functionName, msg->message).c_str());

			// Returning XR_TRUE here will force the calling function to fail
			return (XrBool32)XR_FALSE;
		};
		// start debug utils
		if (xr.ext_xrCreateDebugUtilsMessengerEXT)
			xr.ext_xrCreateDebugUtilsMessengerEXT(xr.instance, &debug_info, &xr.debugMessenger);

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
#if RGL_DX12_AVAILABLE
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
#endif
		}
		else if (currentAPI == RGL::API::Vulkan) {
#if RGL_VK_AVAILABLE
			XrGraphicsRequirementsVulkanKHR vk_reqs{
				.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR,
				.next = nullptr
			};
			PFN_xrGetVulkanGraphicsRequirementsKHR pfn_xrGetVulkanGraphicsRequirementsKHR = nullptr;
			XR_CHECK(xrGetInstanceProcAddr(xr.instance, "PFN_xrGetVulkanGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&pfn_xrGetVulkanGraphicsRequirementsKHR));
			XR_CHECK(pfn_xrGetVulkanGraphicsRequirementsKHR(xr.instance, xr.systemId, &vk_reqs));
			xr.graphicsBinding.vkBinding = XrGraphicsBindingVulkanKHR{
				.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR,
				.next = nullptr,
				.instance = *(VkInstance*)(devicedata.vkData.instance),
				.physicalDevice = *(VkPhysicalDevice*)(devicedata.vkData.physicalDevice),
				.device = *(VkDevice*)(devicedata.vkData.device),
				.queueFamilyIndex = devicedata.vkData.queueFamilyIndex,
				.queueIndex = devicedata.vkData.queueIndex
			};
#endif
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
		int preferred_format;
		int preferred_depth_format;
		if (currentAPI == RGL::API::Direct3D12) {
#if RGL_DX12_AVAILABLE
			preferred_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			preferred_depth_format = DXGI_FORMAT_D32_FLOAT;
#endif
		}
		else if (currentAPI == RGL::API::Vulkan) {
#if RGL_VK_AVAILABLE
			preferred_format = VK_FORMAT_R8G8B8A8_UNORM;
			preferred_depth_format = VK_FORMAT_D32_SFLOAT;
#endif
		}


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
		xr.rglSwapchainImages.resize(view_count);
		xr.rglDepthSwapchainImages.resize(view_count);
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
			xr.rglSwapchainImages[i].resize(swapchain_length);
			XR_CHECK(xrEnumerateSwapchainImages(xr.swapchains[i], swapchain_length, &swapchain_length, (XrSwapchainImageBaseHeader*)xr.swapchainImages[i].data()));

			// convert to RGL texture objects
			for (uint32_t j = 0; j < swapchain_length; j++) {
				auto& img = xr.swapchainImages[i][j];
#if RGL_DX12_AVAILABLE
				xr.rglSwapchainImages[i][j] = std::make_unique<RGL::TextureD3D12>(ComPtr<ID3D12Resource>(img.d3d12Image.texture), RGL::TextureConfig{
						.usage = RGL::TextureUsage::ColorAttachment,
						.aspect = RGL::TextureAspect::HasColor,
						.width = xr.viewConfigurationViews[i].recommendedImageRectWidth,
						.height = xr.viewConfigurationViews[i].recommendedImageRectHeight,
						.format = RGL::TextureFormat::BGRA8_Unorm,
					},
					device
				);
#elif RGL_VK_AVAILABLE
				Fatal("Vk: not implemented");
#endif
			}
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
			xr.rglDepthSwapchainImages[i].resize(depth_swapchain_length);
			XR_CHECK(xrEnumerateSwapchainImages(xr.depth_swapchains[i], depth_swapchain_length, &depth_swapchain_length, (XrSwapchainImageBaseHeader*)xr.depthSwapchainImages[i].data()));

			for (uint32_t j = 0; j < depth_swapchain_length; j++) {
				auto& img = xr.depthSwapchainImages[i][j];
				if (currentAPI == RGL::API::Direct3D12) {
#if RGL_DX12_AVAILABLE
					xr.rglDepthSwapchainImages[i][j] = std::make_unique<RGL::TextureD3D12>(ComPtr<ID3D12Resource>(img.d3d12Image.texture), RGL::TextureConfig{
						.usage = RGL::TextureUsage::DepthStencilAttachment,
						.aspect = RGL::TextureAspect::HasDepth,
						.width = xr.viewConfigurationViews[i].recommendedImageRectWidth,
						.height = xr.viewConfigurationViews[i].recommendedImageRectHeight,
						.format = RGL::TextureFormat::D32SFloat
						},
						device
					);
#endif
				}
				else {
#if RGL_VK_AVAILABLE
					VkImageViewCreateInfo createInfo{
					   .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					   .image = img.vkImage.image,
					   .viewType = VK_IMAGE_VIEW_TYPE_2D,
					   .format = VK_FORMAT_B8G8R8A8_UNORM,
					   .components{
						   .r = VK_COMPONENT_SWIZZLE_IDENTITY, // we don't want any swizzling
						   .g = VK_COMPONENT_SWIZZLE_IDENTITY,
						   .b = VK_COMPONENT_SWIZZLE_IDENTITY,
						   .a = VK_COMPONENT_SWIZZLE_IDENTITY
				   },
					   .subresourceRange{
						   .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,    // mipmap and layer info (we don't want any here)
						   .baseMipLevel = 0,
						   .levelCount = 1,
						   .baseArrayLayer = 0,
						   .layerCount = 1
					   }
					};
					VkImageView imageView;
					vkCreateImageView(*(VkDevice*)(devicedata.vkData.device), &createInfo, nullptr, &imageView);
					xr.rglDepthSwapchainImages[i][j] = std::make_unique<RGL::TextureVk>(imageView, img.vkImage.image, RGL::Dimension{ xr.viewConfigurationViews[i].recommendedImageRectWidth, xr.viewConfigurationViews[i].recommendedImageRectHeight});
#endif
				}
			}
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
			RGL::BufferAccess::Shared
			});
		vertexBuffer->SetBufferData(BasicObjects::Cube::vertices);

		indexBuffer = device->CreateBuffer({
			RGL::BufferConfig::Type::IndexBuffer,
			sizeof(BasicObjects::Cube::indices[0]),
			BasicObjects::Cube::indices,
			RGL::BufferAccess::Shared
			});
		indexBuffer->SetBufferData(BasicObjects::Cube::indices);

		createDepthTexture();

		// create a pipeline layout
		// this describes what we *can* bind to the shader
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
					.stride = sizeof(BasicObjects::Cube::Vertex),
				}},
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
		
		ubo.timeSinceStart = getTimeSeconds();

		// xr frame wait
		XrFrameState frameState{
			.type = XR_TYPE_FRAME_STATE,
			.next = nullptr
		};
		XrFrameWaitInfo frameWaitInfo{
			.type = XR_TYPE_FRAME_WAIT_INFO,
			.next = nullptr
		};
		XR_CHECK(xrWaitFrame(xr.session, &frameWaitInfo, &frameState));

		// get viewproj for the headset
		XrViewLocateInfo viewLocateInfo{
			.type = XR_TYPE_VIEW_LOCATE_INFO,
			.next = nullptr,
			.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
			.displayTime = frameState.predictedDisplayTime,
			.space = xr.space
		};
		uint32_t view_count = xr.viewConfigurationViews.size();
		std::vector<XrView> views{ view_count, {.type = XR_TYPE_VIEW, .next = nullptr } };	// note: avoid making vectors every frame
		XrViewState viewState{
			.type = XR_TYPE_VIEW_STATE,
			.next = nullptr,
		};
		XR_CHECK(xrLocateViews(xr.session, &viewLocateInfo, &viewState, view_count, &view_count, views.data()));
		
		RGL::SwapchainPresentConfig presentConfig{
		};

		swapchain->GetNextImage(&presentConfig.imageIndex);
		swapchainFence->Wait();
		swapchainFence->Reset();

		// begin frame
		XrFrameBeginInfo frameBeginInfo{
			.type = XR_TYPE_FRAME_BEGIN_INFO,
			.next = nullptr,
		};
		XR_CHECK(xrBeginFrame(xr.session, &frameBeginInfo));

		commandBuffer->Reset();
		commandBuffer->Begin();

		auto render = [this](RGL::ITexture* nextimg, RGL::ITexture* depthTexture, glm::mat4 viewProj) {

			auto nextImgSize = nextimg->GetSize();
			renderPass->SetAttachmentTexture(0, nextimg);
			renderPass->SetDepthAttachmentTexture(depthTexture);	// we recreated it so we need to reset it


			commandBuffer->TransitionResource(nextimg, RGL::ResourceLayout::Undefined, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::TransitionPosition::Top);
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
			commandBuffer->SetIndexBuffer(indexBuffer);
			commandBuffer->DrawIndexed(std::size(BasicObjects::Cube::indices));

			commandBuffer->EndRendering();
			commandBuffer->TransitionResource(nextimg, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::ResourceLayout::Present, RGL::TransitionPosition::Bottom);
		};

		for (uint32_t i = 0; i < view_count; i++) {
			// get the swapchain image for both color and depth			

			XrSwapchainImageAcquireInfo color_acquire_info{
				.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO,
				.next = nullptr,
			};
			uint32_t color_acquired_index;
			XR_CHECK(xrAcquireSwapchainImage(xr.swapchains[i], &color_acquire_info, &color_acquired_index));
			XrSwapchainImageWaitInfo waitInfo{
				.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO,
				.next = nullptr,
				.timeout = 1000,
			};
			XR_CHECK(xrWaitSwapchainImage(xr.swapchains[i], &waitInfo));


			XrSwapchainImageAcquireInfo depth_aquire_info{
				.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO,
				.next = nullptr,
			};
			uint32_t depth_acquired_index;
			XR_CHECK(xrAcquireSwapchainImage(xr.depth_swapchains[i], &depth_aquire_info, &depth_acquired_index));
			waitInfo = {
				.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO,
				.next = nullptr,
				.timeout = 1000,
			};
			XR_CHECK(xrWaitSwapchainImage(xr.depth_swapchains[i], &waitInfo));

			xr.projectionViews[i].pose = views[i].pose;
			xr.projectionViews[i].fov = views[i].fov;

			auto genViewMat = [](const XrPosef& pose) {
				return glm::inverse(glm::translate(glm::mat4(1), glm::vec3(pose.position.x, pose.position.y, pose.position.z)) * (glm::toMat4(glm::quat(pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z))));
			};

			auto genProjMat = [this](const XrFovf& fov, float width, float height) {
				return glm::mat4(glm::perspective(fov.angleRight * 2, (float)width / height, nearZ, farZ));
			};
			// calculate the viewproj matrix
			const auto& viewState = views[i];

			ubo.viewProj = genProjMat(xr.projectionViews[i].fov, float(xr.viewConfigurationViews[i].recommendedImageRectWidth), float(xr.viewConfigurationViews[i].recommendedImageRectHeight)) * genViewMat(xr.projectionViews[i].pose);

			render(xr.rglSwapchainImages[i][color_acquired_index].get(), xr.rglDepthSwapchainImages[i][depth_acquired_index].get(), ubo.viewProj);

			XrSwapchainImageReleaseInfo release_info{
				.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO,
				.next = nullptr,
			};
			XR_CHECK(xrReleaseSwapchainImage(xr.swapchains[i], &release_info));
			XrSwapchainImageReleaseInfo depth_release_info{
				.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO,
				.next = nullptr,
			};
			XR_CHECK(xrReleaseSwapchainImage(xr.depth_swapchains[i], &depth_release_info));

		}
		
		auto nextimg = swapchain->ImageAtIndex(presentConfig.imageIndex);

		// render the main screen
		// reuse the last rendered Eye to mirror to the main display
		render(nextimg, depthTexture.get(), ubo.viewProj);
	
		commandBuffer->End();

		RGL::CommitConfig commitconfig{
			.signalFence = swapchainFence,
		};
		commandBuffer->Commit(commitconfig);

		swapchain->Present(presentConfig);

		XrCompositionLayerProjection projectionLayer{
			.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION,
			.next = nullptr,
			.layerFlags = 0,
			.space = xr.space,
			.viewCount = view_count,
			.views = xr.projectionViews.data()
		};
		const XrCompositionLayerBaseHeader* submittedLayers[] = {
			(const XrCompositionLayerBaseHeader* const)&projectionLayer
		};

		XrFrameEndInfo frameEndInfo{
			.type = XR_TYPE_FRAME_END_INFO,
			.next = nullptr,
			.displayTime = frameState.predictedDisplayTime,
			.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
			.layerCount = std::size(submittedLayers),
			.layers = submittedLayers,
		};
		XR_CHECK(xrEndFrame(xr.session, &frameEndInfo));
	}

	void onresize(int width, int height) final {
		createDepthTexture();
	}

	void sampleshutdown() final {
		XR_CHECK(xrEndSession(xr.session));
		XR_CHECK(xrDestroySession(xr.session));

		if (xr.ext_xrDestroyDebugUtilsMessengerEXT != XR_NULL_HANDLE) xr.ext_xrDestroyDebugUtilsMessengerEXT(xr.debugMessenger);

		XR_CHECK(xrDestroyInstance(xr.instance));

		renderPass.reset();
		depthTexture.reset();
		commandBuffer.reset();
		commandQueue.reset();

		vertexBuffer.reset();
		indexBuffer.reset();

		renderPipeline.reset();
	}
};

START_SAMPLE(XR);
