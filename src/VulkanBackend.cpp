#include "pch.h"
#include "Shader.h"
#include "VulkanBackend.h"
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#include <set>
#include <limits>
#include <algorithm>

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
	vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

	if (deviceCount == 0) return std::unexpected("VulkanBackend.cpp | PickPhysicalDevice() | physical device not found");

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(_instance, &deviceCount, physicalDevices.data());

	for (const auto& device : physicalDevices)
	{
		if (*IsPhysicalDeviceSuitable(device))
		{
			_physicalDevice = device;
			break;
		}
	}

	if (_physicalDevice == nullptr) return std::unexpected("VulkanBackend.cpp | PickPhysicalDevice() | no suitable physical device");

}

RETURN(bool) VulkanBackend::IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices indices = *FindQueueFamilies(physicalDevice);
	bool extensionsSupported = *CheckDeviceExtensionSupport(physicalDevice);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapchainSupportDetails swapChainSupport = *QuerySwapchainSupport(physicalDevice);
		swapChainAdequate = !swapChainSupport.surfaceFormats.empty() && !swapChainSupport.presentModes.empty();
	}


	return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}

RETURN(VulkanBackend::QueueFamilyIndices) VulkanBackend::FindQueueFamilies(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices indices;

	unsigned queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;

		unsigned presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, _surface, &presentSupport);

		if (presentSupport) indices.presentFamily = i;
		if (indices.IsComplete()) break;

		i++;
	}

	return indices;
}

RETURN(void) VulkanBackend::CreateDevice()
{
	QueueFamilyIndices indices = *FindQueueFamilies(_physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<unsigned> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

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

	VkDeviceCreateInfo createInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		//.pNext = ,
		//.flags = ,
		.queueCreateInfoCount = (unsigned)queueCreateInfos.size(),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledLayerCount = _enableValidationLayers ? (unsigned)_instanceLayers.size() : 0,
		.ppEnabledLayerNames = _enableValidationLayers ? _instanceLayers.data() : nullptr,
		.enabledExtensionCount = (unsigned)_deviceExtensions.size(),
		.ppEnabledExtensionNames = _deviceExtensions.data(),
		.pEnabledFeatures = &deviceFeatures,
	};

	if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device)) return std::unexpected("VulkanBackend.cpp | CreateDevice() | vkCreateDevice");

	vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
	vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);

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

	if (vkCreateWin32SurfaceKHR(_instance, &createInfo, nullptr, &_surface)) return std::unexpected("VulkanBackend.cpp | CreateSurface() | vkCreateWin32SurfaceKHR");
}

RETURN(void) VulkanBackend::CreateSwapchain()
{
	SwapchainSupportDetails swapchainSupport = *QuerySwapchainSupport(_physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = *ChooseSwapchainSurfaceFormat(swapchainSupport.surfaceFormats);
	VkPresentModeKHR presentMode = *ChooseSwapchainPresentMode(swapchainSupport.presentModes);
	VkExtent2D extent = *ChooseSwapchainExtent(swapchainSupport.surfaceCapabilities);

	unsigned imageCount = swapchainSupport.surfaceCapabilities.minImageCount + 1;

	if (swapchainSupport.surfaceCapabilities.maxImageCount > 0 && imageCount > swapchainSupport.surfaceCapabilities.maxImageCount)imageCount = swapchainSupport.surfaceCapabilities.maxImageCount;

	QueueFamilyIndices indices = *FindQueueFamilies(_physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	VkSwapchainCreateInfoKHR createInfo
	{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		//.pNext = ,
		//.flags = ,
		.surface = _surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = indices.graphicsFamily != indices.presentFamily ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = indices.graphicsFamily != indices.presentFamily ? (unsigned)2 : 0,
		.pQueueFamilyIndices = indices.graphicsFamily != indices.presentFamily ? queueFamilyIndices : nullptr,
		.preTransform = swapchainSupport.surfaceCapabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		//.clipped = ,
		.oldSwapchain = nullptr,
	};

	if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapchain)) return std::unexpected("VulkanBackend.cpp | CreateSwapchain() | vkCreateSwapchainKHR");

	vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);
	_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _swapchainImages.data());

	_swapchainImageFormat = surfaceFormat.format;
	_swapchainExtent = extent;
}

RETURN(void) VulkanBackend::CreateSwapchainImageViews()
{
	_swapchainImageViews.resize(_swapchainImages.size());

	for (size_t i = 0; i < _swapchainImages.size(); i++)
	{
		VkImageViewCreateInfo imageViewCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.image = _swapchainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = _swapchainImageFormat,
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

		if (vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &_swapchainImageViews[i])) return std::unexpected("VulkanBackend.cpp | CreateSwapchainImageViews() | vkCreateImageView");
	}
}

RETURN(void) VulkanBackend::CreateGraphicsPipeline()
{
	return RETURN(void)();
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

std::expected<void, const char*> VulkanBackend::CreateInstance(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions)
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

	if (vkCreateInstance(&instanceCreateInfo, nullptr, &_instance)) return std::unexpected("VulkanBackend.cpp | CreateInstance() | vkCreateInstance");
}

std::expected<bool, const char*> VulkanBackend::Init(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions, unsigned dExtensionCount, const char** dExtensions)
{
	_instanceLayers.resize(_instanceLayers.size() + layerCount);
	for (size_t i = _instanceLayers.size() - layerCount; i < layerCount; i++) _instanceLayers[i] = layers[i];

	_instanceExtensions.resize(_instanceExtensions.size() + extensionCount);
	for (size_t i = _instanceExtensions.size() - extensionCount; i < extensionCount; i++) _instanceExtensions[i] = extensions[i];

	_deviceExtensions.resize(_deviceExtensions.size() + dExtensionCount);
	for (size_t i = _deviceExtensions.size() - dExtensionCount; i < dExtensionCount; i++) _deviceExtensions[i] = dExtensions[i];

	ATTEMPT(CreateInstance(_instanceLayers.size(), _instanceLayers.data(), _instanceExtensions.size(), _instanceExtensions.data()));
	ATTEMPT(CreateSurface());
	ATTEMPT(PickPhysicalDevice());
	ATTEMPT(CreateDevice());
	ATTEMPT(CreateSwapchain());
	ATTEMPT(CreateGraphicsPipeline());

	return true;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddShaders(std::vector<Shader> shaders)
{
	_pipelineShaderStageCreateInfos.resize(shaders.size());

	int i = 0;
	for (auto& shader : shaders)
	{
		_pipelineShaderStageCreateInfos[i] = *shader.GetPipelineShaderStageCreateInfo();
		i++;
	}

	return *this;
}

VkPipeline GraphicsPipelineBuilder::BuildPipeline()
{
	VkPipeline pipeline;

	VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
		.colorAttachmentCount = colorAttachmentCount,
		.pColorAttachmentFormats = pipelineDescription.colorAttachmentFormats.data(),
		.depthAttachmentFormat = pipelineDescription.depthFormat,
		.stencilAttachmentFormat = pipelineDescription.depthFormat
	};

	VkGraphicsPipelineCreateInfo pipelineCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &pipelineRenderingCreateInfo,
		//.flags = ,
		.stageCount = (unsigned)_pipelineShaderStageCreateInfos.size(),
		.pStages = _pipelineShaderStageCreateInfos.data(),
		.pVertexInputState = &_pipelineVertexInputStateCreateInfo,
		.pInputAssemblyState = &_pipelineInputAssemblyStateCreateInfo,
		.pTessellationState = &_pipelineTessellationStateCreateInfo,
		.pViewportState = &_pipelineViewportStateCreateInfo,
		.pRasterizationState = &_pipelineRasterizationStateCreateInfo,
		.pMultisampleState = &_pipelineMultisampleStateCreateInfo,
		.pDepthStencilState = &_pipelineDepthStencilStateCreateInfo,
		.pColorBlendState = &_pipelineColorBlendStateCreateInfo,
		.pDynamicState = &_pipelineDynamicStateCreateInfo,
		.layout = _pipelineLayout,
		//.renderPass = ,
		//.subpass = ,
		//.basePipelineHandle = ,
		//.basePipelineIndex = ,
	};

	//vkCreateGraphicsPipelines(*_vk. , nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &outPipeline);

	return pipeline;
}
