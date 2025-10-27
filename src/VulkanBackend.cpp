#include "pch.h"
#include "VulkanBackend.h"
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#include <set>
#include <limits>
#include <algorithm>
#include <cassert>

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
	QueueFamilyIndices indices = *FindQueueFamilies(_vk.physicalDevice);

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

	VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
		.pNext = nullptr,
		.dynamicRendering = true
	};

	VkDeviceCreateInfo createInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &dynamicRendering,
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

	vkGetDeviceQueue(_vk.device, indices.graphicsFamily.value(), 0, &_vk.graphicsQueue);
	vkGetDeviceQueue(_vk.device, indices.presentFamily.value(), 0, &_vk.presentQueue);

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

	unsigned imageCount = swapchainSupport.surfaceCapabilities.minImageCount + 1;

	if (swapchainSupport.surfaceCapabilities.maxImageCount > 0 && imageCount > swapchainSupport.surfaceCapabilities.maxImageCount)imageCount = swapchainSupport.surfaceCapabilities.maxImageCount;

	QueueFamilyIndices indices = *FindQueueFamilies(_vk.physicalDevice);
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

	if (vkCreateSwapchainKHR(_vk.device, &createInfo, nullptr, &_swapchain)) return std::unexpected("VulkanBackend.cpp | CreateSwapchain() | vkCreateSwapchainKHR");

	vkGetSwapchainImagesKHR(_vk.device, _swapchain, &imageCount, nullptr);
	_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(_vk.device, _swapchain, &imageCount, _swapchainImages.data());

	_swapchainImageFormat = surfaceFormat.format;
	_swapchainExtent = extent;

	return {};
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

		if (vkCreateImageView(_vk.device, &imageViewCreateInfo, nullptr, &_swapchainImageViews[i])) return std::unexpected("VulkanBackend.cpp | CreateSwapchainImageViews() | vkCreateImageView");
	}
}

void VulkanBackend::CreateGraphicsPipeline()
{
	GraphicsPipelineBuilder pipelineBuilder(_vk);

	std::vector<std::pair<std::string, ShaderType>> shaders
	{
		{ "Pixel Shader", ShaderType::FRAGMENT },
		{ "VertexShader", ShaderType::VERTEX }
	};

	std::vector<VertexInputDescription> vertexInputDescriptions
	{
		VertexInputDescription
		{
			.binding = 0,
			.location = 0,
			.stride = sizeof(float) * 3,
			.format = PipelineFormat::FLOAT3,
			.offset = 0,
		},
		VertexInputDescription
		{
			.binding = 1,
			.location = 1,
			.stride = sizeof(float) * 4,
			.format = PipelineFormat::COLOR,
			.offset = 12,
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
			(PipelineFormat)_swapchainImageFormat
		}
	};

	Attempt(pipelineBuilder.AddShaders(shaders).AddVertexBindingDescriptions(vertexInputDescriptions).AddPipelineAttachments(pipelineAttachments).SetRenderingInfo(renderInfo).BuildPipeline());
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
	CreateGraphicsPipeline();

	return true;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddShaders(std::vector<std::pair<std::string, ShaderType>> shader)
{
	_shaders.reserve(shader.size());
	_pipelineShaderStageCreateInfos.resize(shader.size());

	int i = 0;
	for (auto& s : shader)
	{
		_shaders.emplace_back(_vk->device, s.first, s.second);
		_pipelineShaderStageCreateInfos[i] = Attempt(_shaders[i].GetPipelineShaderStageCreateInfo());
		i++;
	}

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddVertexBindingDescriptions(std::vector<VertexInputDescription> inputDescriptions)
{
	_inputBindingDescriptions.resize(inputDescriptions.size());
	_inputAttributeDescriptions.resize(inputDescriptions.size());

	int i = 0;
	for (auto inputDescription : inputDescriptions)
	{
		VkVertexInputBindingDescription inputBinding
		{
			.binding = inputDescription.binding,
			.stride = inputDescription.stride,
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};

		VkVertexInputAttributeDescription inputAttribute
		{
			.location = inputDescription.location,
			.binding = inputDescription.binding,
			.format = (VkFormat)inputDescription.format,
			.offset = inputDescription.offset
		};

		_inputBindingDescriptions[i] = inputBinding;
		_inputAttributeDescriptions[i] = inputAttribute;
		i++;
	}

	_pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = (unsigned)_inputBindingDescriptions.size();
	_pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = _inputBindingDescriptions.data();
	_pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = (unsigned)_inputAttributeDescriptions.size();
	_pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = _inputAttributeDescriptions.data();

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetTopology(Topology topology)
{
	_pipelineInputAssemblyStateCreateInfo.topology = (VkPrimitiveTopology)topology;

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::BuildTessellationState(unsigned patchControlPoints)
{
	_pipelineTessellationStateCreateInfo.patchControlPoints = patchControlPoints;

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::BuildViewportState(unsigned width, unsigned height)
{
	_viewport =
	{
		.x = 0,
		.y = 0,
		.width = static_cast<float>(width),
		.height = static_cast<float>(height),
		.minDepth = 0,
		.maxDepth = 1
	};

	_scissor =
	{
		.offset = {0, 0},
		.extent = {width, height}
	};

	_pipelineViewportStateCreateInfo.pViewports = &_viewport;
	_pipelineViewportStateCreateInfo.pScissors = &_scissor;

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetRasterizationState(RasterStateInfo rasterStateInfo)
{
	_pipelineRasterizationStateCreateInfo.polygonMode = (VkPolygonMode)rasterStateInfo.polygonMode;
	_pipelineRasterizationStateCreateInfo.cullMode = (VkCullModeFlags)rasterStateInfo.cullMode;
	_pipelineRasterizationStateCreateInfo.frontFace = (VkFrontFace)rasterStateInfo.frontFace;

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddDepthTest()
{
	_pipelineDepthStencilStateCreateInfo.depthTestEnable = true;

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddDepthWrite()
{
	_pipelineDepthStencilStateCreateInfo.depthWriteEnable = true;

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddStencilTest()
{
	_pipelineDepthStencilStateCreateInfo.stencilTestEnable = true;

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddPipelineAttachments(std::vector<PipelineAttachment> pipelineAttachments)
{
	_blendAttachments.resize(pipelineAttachments.size());

	int i = 0;
	for (auto pipelineAttachment : pipelineAttachments)
	{
		VkPipelineColorBlendAttachmentState attachmentState
		{
			.blendEnable = pipelineAttachment.blend,
			.srcColorBlendFactor = (VkBlendFactor)pipelineAttachment.colorSource,
			.dstColorBlendFactor = (VkBlendFactor)pipelineAttachment.colorDestination,
			.colorBlendOp = (VkBlendOp)pipelineAttachment.colorOperation,
			.srcAlphaBlendFactor = (VkBlendFactor)pipelineAttachment.alphaSource,
			.dstAlphaBlendFactor = (VkBlendFactor)pipelineAttachment.alphaDestination,
			.alphaBlendOp = (VkBlendOp)pipelineAttachment.alphaOperation,
			.colorWriteMask = pipelineAttachment.writeMask,
		};

		_blendAttachments[i] = attachmentState;
		i++;
	}

	_pipelineColorBlendStateCreateInfo.attachmentCount = (unsigned)_blendAttachments.size();
	_pipelineColorBlendStateCreateInfo.pAttachments = _blendAttachments.data();

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
{
	_pipelineDescriptorSetLayouts.emplace_back(descriptorSetLayout);

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddPushConstantRange(VkPushConstantRange pushConstantRange)
{
	_pipelinePushConstantRanges.emplace_back(pushConstantRange);

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetRenderingInfo(RenderingInfo renderingInfo)
{
	_colorAttachmentFormats.resize(renderingInfo.colorAttachmentFormats.size());

	int i = 0;
	for (auto format : renderingInfo.colorAttachmentFormats)
	{
		_colorAttachmentFormats[i] = (VkFormat)format;
		i++;
	}

	_pipelineRenderingCreateInfo.colorAttachmentCount = (unsigned)_colorAttachmentFormats.size();
	_pipelineRenderingCreateInfo.pColorAttachmentFormats = (VkFormat*)_colorAttachmentFormats.data();
	_pipelineRenderingCreateInfo.depthAttachmentFormat = (VkFormat)renderingInfo.depthStencilFormat;
	_pipelineRenderingCreateInfo.stencilAttachmentFormat = (VkFormat)renderingInfo.depthStencilFormat;

	return *this;
}

RETURN(VkPipeline) GraphicsPipelineBuilder::BuildPipeline()
{
	VkPipeline pipeline = nullptr;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		//.pNext = ,
		//.flags = ,
		.setLayoutCount = (unsigned)_pipelineDescriptorSetLayouts.size(),
		.pSetLayouts = _pipelineDescriptorSetLayouts.data(),
		.pushConstantRangeCount = (unsigned)_pipelinePushConstantRanges.size(),
		.pPushConstantRanges = _pipelinePushConstantRanges.data(),
	};

	if (vkCreatePipelineLayout(_vk->device, &pipelineLayoutCreateInfo, nullptr, &_pipelineLayout)) return std::unexpected("VulkanBackend.cpp | BuildPipeline() | vkCreatePipelineLayout");

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &_pipelineRenderingCreateInfo,
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

	if (vkCreateGraphicsPipelines(_vk->device, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline)) return std::unexpected("VulkanBackend.cpp | BuildPipeline() | vkCreateGraphicsPipelines");

	return pipeline;
}
