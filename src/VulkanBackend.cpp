#include "pch.h"
#include "VulkanBackend.h"
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#include <set>
#include <limits>
#include <algorithm>
#include <cassert>
#include "PipelineBuilder.h"
#include <memory>

RETURN(bool, const char*) VulkanBackend::CheckCompatibility(const char** instanceExtensions, const char** deviceExtensions)
{
	const char* use_surface = VK_KHR_SURFACE_EXTENSION_NAME;
	const char* use_swapchain = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	//	const char* platform_surface = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
	const char* platform_device = nullptr;

	return false;
}

RETURN(bool, const char*) VulkanBackend::CheckLayerSupport()
{
	unsigned layerCount;

	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : _instanceLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound) return false;
	}

	return true;
}

RETURN(void) VulkanBackend::PickPhysicalDevice()
{
	unsigned deviceCount = 0;
	vkEnumeratePhysicalDevices(_vk.instance, &deviceCount, nullptr);

	if (deviceCount == 0) return std::unexpected("VulkanBackend.cpp | PickPhysicalDevice() | physical device not found");

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(_vk.instance, &deviceCount, physicalDevices.data());

	for (const auto& device : physicalDevices)
	{
		if (*IsPhysicalDeviceSuitable(device))
		{
			_vk.physicalDevice = device;
			break;
		}
	}

	if (_vk.physicalDevice == nullptr) return std::unexpected("VulkanBackend.cpp | PickPhysicalDevice() | no suitable physical device");

	return {};
}

RETURN(bool) VulkanBackend::IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice)
{
	_vk.queueFamilyIndices = *FindQueueFamilies(physicalDevice);
	bool extensionsSupported = *CheckDeviceExtensionSupport(physicalDevice);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapchainSupportDetails swapChainSupport = *QuerySwapchainSupport(physicalDevice);
		swapChainAdequate = !swapChainSupport.surfaceFormats.empty() && !swapChainSupport.presentModes.empty();
	}


	return _vk.queueFamilyIndices.IsComplete() && extensionsSupported && swapChainAdequate;
}

RETURN(VulkanContext::QueueFamilyIndices) VulkanBackend::FindQueueFamilies(VkPhysicalDevice physicalDevice)
{
	unsigned queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) _vk.queueFamilyIndices.graphicsFamily = i;

		unsigned presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, _surface, &presentSupport);

		if (presentSupport) _vk.queueFamilyIndices.presentFamily = i;
		if (_vk.queueFamilyIndices.IsComplete()) break;

		i++;
	}

	return _vk.queueFamilyIndices;
}

RETURN(void) VulkanBackend::CreateDevice()
{
	_vk.queueFamilyIndices = *FindQueueFamilies(_vk.physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<unsigned> uniqueQueueFamilies = { _vk.queueFamilyIndices.graphicsFamily.value(), _vk.queueFamilyIndices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (unsigned queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.queueFamilyIndex = queueFamily,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority,
		};
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeatureExt
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
		//.pNext = , 
		.descriptorBuffer = true,
		//.descriptorBufferCaptureReplay = , 
		//.descriptorBufferImageLayoutIgnored = , 
		//.descriptorBufferPushDescriptors = , 
	};

	VkPhysicalDeviceVulkan12Features vulkan12features
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = &descriptorBufferFeatureExt,
		//.samplerMirrorClampToEdge = ,
		//.drawIndirectCount = ,
		//.storageBuffer8BitAccess = ,
		//.uniformAndStorageBuffer8BitAccess = ,
		//.storagePushConstant8 = ,
		//.shaderBufferInt64Atomics = ,
		//.shaderSharedInt64Atomics = ,
		//.shaderFloat16 = ,
		//.shaderInt8 = ,
		//.descriptorIndexing = ,
		//.shaderInputAttachmentArrayDynamicIndexing = ,
		//.shaderUniformTexelBufferArrayDynamicIndexing = ,
		//.shaderStorageTexelBufferArrayDynamicIndexing = ,
		//.shaderUniformBufferArrayNonUniformIndexing = ,
		//.shaderSampledImageArrayNonUniformIndexing = ,
		//.shaderStorageBufferArrayNonUniformIndexing = ,
		//.shaderStorageImageArrayNonUniformIndexing = ,
		//.shaderInputAttachmentArrayNonUniformIndexing = ,
		//.shaderUniformTexelBufferArrayNonUniformIndexing = ,
		//.shaderStorageTexelBufferArrayNonUniformIndexing = ,
		//.descriptorBindingUniformBufferUpdateAfterBind = ,
		//.descriptorBindingSampledImageUpdateAfterBind = ,
		//.descriptorBindingStorageImageUpdateAfterBind = ,
		//.descriptorBindingStorageBufferUpdateAfterBind = ,
		//.descriptorBindingUniformTexelBufferUpdateAfterBind = ,
		//.descriptorBindingStorageTexelBufferUpdateAfterBind = ,
		//.descriptorBindingUpdateUnusedWhilePending = ,
		//.descriptorBindingPartiallyBound = ,
		//.descriptorBindingVariableDescriptorCount = ,
		//.runtimeDescriptorArray = ,
		//.samplerFilterMinmax = ,
		//.scalarBlockLayout = ,
		//.imagelessFramebuffer = ,
		//.uniformBufferStandardLayout = ,
		//.shaderSubgroupExtendedTypes = ,
		//.separateDepthStencilLayouts = ,
		//.hostQueryReset = ,
		//.timelineSemaphore = ,
		.bufferDeviceAddress = true,
		//.bufferDeviceAddressCaptureReplay = ,
		//.bufferDeviceAddressMultiDevice = ,
		//.vulkanMemoryModel = ,
		//.vulkanMemoryModelDeviceScope = ,
		//.vulkanMemoryModelAvailabilityVisibilityChains = ,
		//.shaderOutputViewportIndex = ,
		//.shaderOutputLayer = ,
		//.subgroupBroadcastDynamicId = ,
	};

	VkPhysicalDeviceVulkan13Features vulkan13features
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &vulkan12features,
		//.robustImageAccess = ,
		//.inlineUniformBlock = ,
		//.descriptorBindingInlineUniformBlockUpdateAfterBind = ,
		//.pipelineCreationCacheControl = ,
		//.privateData = ,
		//.shaderDemoteToHelperInvocation = ,
		//.shaderTerminateInvocation = ,
		//.subgroupSizeControl = ,
		//.computeFullSubgroups = ,
		.synchronization2 = true,
		//.textureCompressionASTC_HDR = ,
		//.shaderZeroInitializeWorkgroupMemory = ,
		.dynamicRendering = true,
		//.shaderIntegerDotProduct = ,
		//.maintenance4 = ,
	};

	VkDeviceCreateInfo createInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &vulkan13features,
		//.flags = ,
		.queueCreateInfoCount = (unsigned)queueCreateInfos.size(),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledLayerCount = _enableValidationLayers ? (unsigned)_instanceLayers.size() : 0,
		.ppEnabledLayerNames = _enableValidationLayers ? _instanceLayers.data() : nullptr,
		.enabledExtensionCount = (unsigned)_deviceExtensions.size(),
		.ppEnabledExtensionNames = _deviceExtensions.data(),
		.pEnabledFeatures = &deviceFeatures,
	};

	if (vkCreateDevice(_vk.physicalDevice, &createInfo, nullptr, &_vk.device)) return std::unexpected("VulkanBackend.cpp | CreateDevice() | vkCreateDevice");

	vkGetDeviceQueue(_vk.device, _vk.queueFamilyIndices.graphicsFamily.value(), 0, &_vk.graphicsQueue);
	vkGetDeviceQueue(_vk.device, _vk.queueFamilyIndices.presentFamily.value(), 0, &_vk.presentQueue);

	return {};
}

RETURN(void) VulkanBackend::CreateSurface()
{
	VkWin32SurfaceCreateInfoKHR createInfo
	{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		//.pNext = , 
		//.flags = , 
		.hinstance = nullptr,
		.hwnd = (HWND)ImgnWindow::GetInstance().GetWindowHandle(),
	};

	if (vkCreateWin32SurfaceKHR(_vk.instance, &createInfo, nullptr, &_surface)) return std::unexpected("VulkanBackend.cpp | CreateSurface() | vkCreateWin32SurfaceKHR");

	return {};
}

RETURN(void) VulkanBackend::CreateSwapchain()
{
	SwapchainSupportDetails swapchainSupport = *QuerySwapchainSupport(_vk.physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = *ChooseSwapchainSurfaceFormat(swapchainSupport.surfaceFormats);
	VkPresentModeKHR presentMode = *ChooseSwapchainPresentMode(swapchainSupport.presentModes);
	VkExtent2D extent = *ChooseSwapchainExtent(swapchainSupport.surfaceCapabilities);

	_maxFrames = swapchainSupport.surfaceCapabilities.minImageCount + 1;

	if (swapchainSupport.surfaceCapabilities.maxImageCount > 0 && _maxFrames > swapchainSupport.surfaceCapabilities.maxImageCount) _maxFrames = swapchainSupport.surfaceCapabilities.maxImageCount;

	_vk.queueFamilyIndices = *FindQueueFamilies(_vk.physicalDevice);
	uint32_t queueFamilyIndices[] = { _vk.queueFamilyIndices.graphicsFamily.value(), _vk.queueFamilyIndices.presentFamily.value() };

	VkSwapchainCreateInfoKHR createInfo
	{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		//.pNext = ,
		//.flags = ,
		.surface = _surface,
		.minImageCount = _maxFrames,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = _vk.queueFamilyIndices.graphicsFamily != _vk.queueFamilyIndices.presentFamily ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = _vk.queueFamilyIndices.graphicsFamily != _vk.queueFamilyIndices.presentFamily ? (unsigned)2 : 0,
		.pQueueFamilyIndices = _vk.queueFamilyIndices.graphicsFamily != _vk.queueFamilyIndices.presentFamily ? queueFamilyIndices : nullptr,
		.preTransform = swapchainSupport.surfaceCapabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		//.clipped = ,
		.oldSwapchain = nullptr,
	};

	if (vkCreateSwapchainKHR(_vk.device, &createInfo, nullptr, &_swapchain)) return std::unexpected("VulkanBackend.cpp | CreateSwapchain() | vkCreateSwapchainKHR");

	vkGetSwapchainImagesKHR(_vk.device, _swapchain, &_maxFrames, nullptr);
	_vk.swapchainImages.resize(_maxFrames);
	vkGetSwapchainImagesKHR(_vk.device, _swapchain, &_maxFrames, _vk.swapchainImages.data());
	
	_vk.swapchainFormat = (PipelineFormat)surfaceFormat.format;
	_vk.swapchainExtent = extent;
	//_swapchainExtent = extent;

	return {};
}

RETURN(void) VulkanBackend::CreateSwapchainImageViews()
{
	_vk.swapchainImageViews.resize(_maxFrames);

	for (size_t i = 0; i < _vk.swapchainImages.size(); i++)
	{
		VkImageViewCreateInfo imageViewCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.image = _vk.swapchainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = (VkFormat)_vk.swapchainFormat,
			.components
			{
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};

		if (vkCreateImageView(_vk.device, &imageViewCreateInfo, nullptr, &_vk.swapchainImageViews[i])) return std::unexpected("VulkanBackend.cpp | CreateSwapchainImageViews() | vkCreateImageView");
	}

	return {};
}

void VulkanBackend::CreateGraphicsPipeline()
{
	GraphicsPipelineBuilder pipelineBuilder(_vk);

	std::vector<std::pair<std::string, ShaderType>> shaders
	{
		{ "PixelShader", ShaderType::FRAGMENT },
		{ "VertexShader", ShaderType::VERTEX }
	};

	std::vector<VertexInputDescription> vertexInputDescriptions
	{
		VertexInputDescription
		{
			.binding = 0,
			.location = 0,
			.stride = sizeof(float) * 2,
			.format = PipelineFormat::FLOAT2,
			.offset = 0,
		},
		VertexInputDescription
		{
			.binding = 1,
			.location = 1,
			.stride = sizeof(unsigned char) * 4,
			.format = PipelineFormat::COLOR,
			.offset = 0,
		},
	};

	std::vector<PipelineAttachment> pipelineAttachments
	{
		PipelineAttachment{}
	};

	RenderingInfo renderInfo
	{
		.colorAttachmentFormats
		{
			_vk.swapchainFormat
		},
		.depthStencilFormat = _vk.depthFormat
	};

	_vk.pipeline = Attempt(pipelineBuilder.AddShaders(shaders).AddVertexBindingDescriptions(vertexInputDescriptions).AddDepthTest().AddDepthWrite().AddPipelineAttachments(pipelineAttachments).SetRenderingInfo(renderInfo).AddPushConstantRange(VkPushConstantRange{
	.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, // or VERTEX_BIT|FRAGMENT_BIT if both use it
	.offset = 0,
	.size = sizeof(float)}).BuildPipeline(_vk.pipelineLayout));
}

void VulkanBackend::InitDepthTexture()
{
	//_vk.depth = std::make_unique<Texture>(_vk);
	Texture depth(_vk);

	VkFormat depthFormat;
	std::vector<VkFormat> formats =
	{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for (size_t i = 0; i < formats.size(); i++)
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(_vk.physicalDevice, formats[i], &formatProperties);

		if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			depthFormat = formats[i];
			break;
		}
	}

	depth.CreateImage({ _vk.win.GetWidth(), _vk.win.GetHeight(), 1 }, 1, SampleCount::SAMPLE_1BIT, (PipelineFormat)depthFormat, ImageTiling::OPTIMAL, ImageUsage::DEPTH_STENCIL, MemoryFlags::GPU).CreateImageView(ImageAspect::DEPTH | ImageAspect::STENCIL).TransitionImageLayout(ImageLayout::DEPTH_STENCIL);
	_vk.depthImage = depth.image;
	_vk.depthImageView = depth.imageView;
	_vk.depthFormat = (PipelineFormat)depthFormat;
	depth.owns = false;
}

RETURN(bool) VulkanBackend::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
	unsigned extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

RETURN(VulkanBackend::SwapchainSupportDetails) VulkanBackend::QuerySwapchainSupport(VkPhysicalDevice physicalDevice)
{
	SwapchainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, _surface, &details.surfaceCapabilities);

	unsigned formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.surfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, details.surfaceFormats.data());
	}

	unsigned presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

RETURN(VkSurfaceFormatKHR) VulkanBackend::ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return availableFormat;
	}

	return availableFormats[0];
}

RETURN(VkPresentModeKHR) VulkanBackend::ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) return availablePresentMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

RETURN(VkExtent2D) VulkanBackend::ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return surfaceCapabilities.currentExtent;
	else
	{
		unsigned width = ImgnWindow::GetInstance().GetWidth(), height = ImgnWindow::GetInstance().GetHeight();

		VkExtent2D actualExtent
		{
			.width = width,
			.height = height
		};

		actualExtent.width = std::clamp(actualExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

		return actualExtent;
	}
}

RETURN(void) VulkanBackend::CreateInstance(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions)
{
	if (_enableValidationLayers && !*CheckLayerSupport()) return std::unexpected("VulkanBackend.cpp | CreateInstance() | instance layers requested, but not available");

	VkApplicationInfo applicationInfo
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Imagination",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Imagination",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_3
	};

	VkInstanceCreateInfo instanceCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &applicationInfo,
		.enabledLayerCount = (unsigned)_instanceLayers.size(),
		.ppEnabledLayerNames = _instanceLayers.data(),
		.enabledExtensionCount = (unsigned)_instanceExtensions.size(),
		.ppEnabledExtensionNames = _instanceExtensions.data()
	};
	
	if (vkCreateInstance(&instanceCreateInfo, nullptr, &_vk.instance)) return std::unexpected("VulkanBackend.cpp | CreateInstance() | vkCreateInstance");

	return {};
}

RETURN(bool) VulkanBackend::Init(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions, unsigned dExtensionCount, const char** dExtensions)
{
	_instanceLayers.resize(_instanceLayers.size() + layerCount);
	for (size_t i = _instanceLayers.size() - layerCount; i < layerCount; i++) _instanceLayers[i] = layers[i];

	_instanceExtensions.resize(_instanceExtensions.size() + extensionCount);
	for (size_t i = _instanceExtensions.size() - extensionCount; i < extensionCount; i++) _instanceExtensions[i] = extensions[i];

	_deviceExtensions.resize(_deviceExtensions.size() + dExtensionCount);
	for (size_t i = _deviceExtensions.size() - dExtensionCount; i < dExtensionCount; i++) _deviceExtensions[i] = dExtensions[i];

	std::filesystem::create_directories("../Shaders/SPV");

	Attempt(CreateInstance(_instanceLayers.size(), _instanceLayers.data(), _instanceExtensions.size(), _instanceExtensions.data()));
	Attempt(CreateSurface());
	Attempt(PickPhysicalDevice());
	Attempt(CreateDevice());
	Attempt(CreateSwapchain());
	Attempt(CreateSwapchainImageViews());

	_swapchainCommandBuffers.resize(_maxFrames);
	_imageAvailableSemaphores.resize(_maxFrames);
	_renderFinishedSemaphores.resize(_maxFrames);
	_renderingFences.resize(_maxFrames);

	VkSemaphoreCreateInfo semaphoreCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		//.pNext = ,
		//.flags = ,
	};

	VkFenceCreateInfo fenceCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		//.pNext = ,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	VkCommandPoolCreateInfo commandPoolCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		//.pNext = ,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = _vk.queueFamilyIndices.graphicsFamily.value(),
	};

	vkCreateCommandPool(_vk.device, &commandPoolCreateInfo, nullptr, &_vk.commandPool);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		//.pNext = ,
		.commandPool = _vk.commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = _maxFrames,
	};

	vkAllocateCommandBuffers(_vk.device, &commandBufferAllocateInfo, _swapchainCommandBuffers.data());
	
	for (size_t i = 0; i < _maxFrames; i++)
	{
		vkCreateSemaphore(_vk.device, &semaphoreCreateInfo, nullptr, &_imageAvailableSemaphores[i]);
		vkCreateSemaphore(_vk.device, &semaphoreCreateInfo, nullptr, &_renderFinishedSemaphores[i]);
		vkCreateFence(_vk.device, &fenceCreateInfo, nullptr, &_renderingFences[i]);
	}

	InitDepthTexture();
	CreateGraphicsPipeline();

	return true;
}

RETURN(VkCommandBuffer) VulkanBackend::StartFrame()
{
	//Frame is now Locked
	_frameLocked = true;

	//Wait for Queue to be ready
	//if (m_PrevFrame != m_vk.currentFrame)
	vkWaitForFences(_vk.device, 1, &_renderingFences[_vk.currentFrame], VK_TRUE, ~(static_cast<uint64_t>(0)));

	vkAcquireNextImageKHR(_vk.device, _swapchain,~(0ull), _imageAvailableSemaphores[_vk.currentFrame], VK_NULL_HANDLE, &_vk.targetFrame);

	VkCommandBuffer commandBuffer = _swapchainCommandBuffers[_vk.currentFrame];

	VkCommandBufferBeginInfo commandBufferBeginInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		//.pNext = ,
		.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
		//.pInheritanceInfo = ,
	};

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

	VkViewport viewport
	{
		.x = 0,
		.y = 0,
		.width = (float)_vk.win.GetWidth(),
		.height = (float)_vk.win.GetHeight(),
		.minDepth = 0,
		.maxDepth = 1,
	};

	VkRect2D scissor =
	{
		.offset
		{
			.x = 0,
			.y = 0
		},
		.extent = { _vk.win.GetWidth(), _vk.win.GetHeight() }
	};

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	if (_initialFrames)
	{
		VkImageMemoryBarrier2 colorImageMemoryBarrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			//.pNext = ,
			.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			//.srcQueueFamilyIndex = ,
			//.dstQueueFamilyIndex = ,
			.image = _vk.swapchainImages[_vk.targetFrame],
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VkImageMemoryBarrier2 depthImageMemoryBarrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			//.pNext = ,
			.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
			.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			//.srcQueueFamilyIndex = ,
			//.dstQueueFamilyIndex = ,
			.image = _vk.swapchainImages[_vk.targetFrame],
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VkDependencyInfo dependencyInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			//.pNext = ,
			//.dependencyFlags = ,
			//.memoryBarrierCount = ,
			//.pMemoryBarriers = ,
			//.bufferMemoryBarrierCount = ,
			//.pBufferMemoryBarriers = ,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &colorImageMemoryBarrier,
		};

		vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
		
		_frame++;
		if (_frame >= 3) _initialFrames = false;
	}
	else
	{
		VkImageMemoryBarrier2 colorImageMemoryBarrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			//.pNext = ,
			.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			//.srcQueueFamilyIndex = ,
			//.dstQueueFamilyIndex = ,
			.image = _vk.swapchainImages[_vk.targetFrame],
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VkDependencyInfo dependencyInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			//.pNext = ,
			//.dependencyFlags = ,
			//.memoryBarrierCount = ,
			//.pMemoryBarriers = ,
			//.bufferMemoryBarrierCount = ,
			//.pBufferMemoryBarriers = ,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &colorImageMemoryBarrier,
		};

		vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
	}

	//VkRenderingAttachmentInfoKHR swapchainRenderingAttachmentInfo
	//{
	//	.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
	//	.imageView = _vk.swapchainImageViews[_vk.targetFrame],
	//	.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
	//	.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
	//	.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
	//	.clearValue
	//	{
	//		.color = {0, 0, 0, 1},
	//		//.depthStencil = ,
	//	}
	//};

	//VkRenderingAttachmentInfoKHR swapchainDepthRenderingAttachmentInfo
	//{
	//	.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
	//	.imageView = _vk.depthImageView,
	//	.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
	//	.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
	//	.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
	//	.clearValue
	//	{
	//		//.color = ,
	//		.depthStencil = { 1.f, 0},
	//	}
	//};

	//VkRenderingInfo renderingInfo
	//{
	//	.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
	//	.renderArea
	//	{
	//		.offset = { 0, 0 },
	//		.extent = { _vk.win.GetWidth(), _vk.win.GetHeight() }
	//	},
	//	.layerCount = 1,
	//	.colorAttachmentCount = 1, //(unsigned)colorRenderingAttachmentInfos.size(),
	//	.pColorAttachments = &swapchainRenderingAttachmentInfo,//colorRenderingAttachmentInfos.data(),
	//	.pDepthAttachment = &swapchainDepthRenderingAttachmentInfo
	//};

	//vkCmdBeginRendering(commandBuffer, &renderingInfo);
	//vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vk.pipeline);

	return commandBuffer;
}

RETURN(void) VulkanBackend::EndFrame(VkCommandBuffer commandBuffer)
{
	//vkCmdEndRendering(commandBuffer);

	{
		VkImageMemoryBarrier2 presentImageMemoryBarrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			//.pNext = ,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_NONE,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			//.srcQueueFamilyIndex = ,
			//.dstQueueFamilyIndex = ,
			.image = _vk.swapchainImages[_vk.targetFrame],
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VkDependencyInfo dependencyInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			//.pNext = ,
			//.dependencyFlags = ,
			//.memoryBarrierCount = ,
			//.pMemoryBarriers = ,
			//.bufferMemoryBarrierCount = ,
			//.pBufferMemoryBarriers = ,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &presentImageMemoryBarrier,
		};

		vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
	}

	vkEndCommandBuffer(commandBuffer);

	//Setup the Semaphores and Command Buffer to be sent into Queue Submit
	VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_vk.currentFrame] }, signalSemaphores[] = { _renderFinishedSemaphores[_vk.targetFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkCommandBuffer commandBuffers[] = { _swapchainCommandBuffers[_vk.currentFrame] };

	//Setup the Queue Submit Info
	VkSubmitInfo submitInfo
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		//.pNext = ,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = waitSemaphores,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = commandBuffers,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signalSemaphores,
	};

	//Reset the Fence
	vkResetFences(_vk.device, 1, &_renderingFences[_vk.currentFrame]);
	vkQueueSubmit(_vk.graphicsQueue, 1, &submitInfo, _renderingFences[_vk.currentFrame]);

	VkSwapchainKHR swapchains[] = { _swapchain };
	VkPresentInfoKHR presentInfo
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		//.pNext = ,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signalSemaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &_vk.targetFrame,
		.pResults = nullptr,
	};
	vkQueuePresentKHR(_vk.presentQueue, &presentInfo);

	if (++_vk.currentFrame >= _maxFrames) _vk.currentFrame = 0;

	_frameLocked = false;

	return {};
}