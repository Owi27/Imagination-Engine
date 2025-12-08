#include "pch.h"
#include "Swapchain.h"
#include "VulkanCtx.h"

void Swapchain::CreateSwapchain()
{
	SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(VkCtx::Instance().physicalDevice, VkCtx::Instance().surface);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapchainSurfaceFormat(swapchainSupport.surfaceFormats);
	VkPresentModeKHR presentMode = ChooseSwapchainPresentMode(swapchainSupport.presentModes);
	VkExtent2D extent = ChooseSwapchainExtent(swapchainSupport.surfaceCapabilities);

	VkCtx::Instance().maxFrame = swapchainSupport.surfaceCapabilities.minImageCount + 1;

	if (swapchainSupport.surfaceCapabilities.maxImageCount > 0 && VkCtx::Instance().maxFrame > swapchainSupport.surfaceCapabilities.maxImageCount) VkCtx::Instance().maxFrame = swapchainSupport.surfaceCapabilities.maxImageCount;

	VkCtx::Instance().queueFamilyIndices = FindQueueFamilies(VkCtx::Instance().physicalDevice);
	uint32_t queueFamilyIndices[] = { VkCtx::Instance().queueFamilyIndices.graphicsFamily.value(), VkCtx::Instance().queueFamilyIndices.presentFamily.value() };

	VkSwapchainCreateInfoKHR createInfo
	{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		//.pNext = ,
		//.flags = ,
		.surface = VkCtx::Instance().surface,
		.minImageCount = VkCtx::Instance().maxFrame,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VkCtx::Instance().queueFamilyIndices.graphicsFamily != VkCtx::Instance().queueFamilyIndices.presentFamily ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = VkCtx::Instance().queueFamilyIndices.graphicsFamily != VkCtx::Instance().queueFamilyIndices.presentFamily ? (unsigned)2 : 0,
		.pQueueFamilyIndices = VkCtx::Instance().queueFamilyIndices.graphicsFamily != VkCtx::Instance().queueFamilyIndices.presentFamily ? queueFamilyIndices : nullptr,
		.preTransform = swapchainSupport.surfaceCapabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		//.clipped = ,
		.oldSwapchain = nullptr,
	};

	vkCreateSwapchainKHR(VkCtx::Instance().device, &createInfo, nullptr, &swapchain);

	vkGetSwapchainImagesKHR(VkCtx::Instance().device, swapchain, &VkCtx::Instance().maxFrame, nullptr);

	std::vector<VkImage> swapImages(VkCtx::Instance().maxFrame);

	vkGetSwapchainImagesKHR(VkCtx::Instance().device, swapchain, &VkCtx::Instance().maxFrame, swapImages.data());
	swapchainImages.resize(VkCtx::Instance().maxFrame);

	swapchainFormat = surfaceFormat.format;
	for (int i = 0; i < VkCtx::Instance().maxFrame; i++)
	{
		swapchainImages[i] = std::make_unique<Texture>();
		swapchainImages[i]->image = std::move(swapImages[i]);
		swapchainImages[i]->format = (PipelineFormat)swapchainFormat;
	}
	swapchainExtent = { extent.width, extent.height, 1 };

	VkCtx::Instance().swapchainExtent = swapchainExtent;
}

void Swapchain::CreateSwapchainImageViews()
{
	for (auto& swapchainImage : swapchainImages)
	{
		swapchainImage->CreateImageView(ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
	}
}

void Swapchain::CreateCommandBuffers()
{
	swapchainCommandBuffers.resize(VkCtx::Instance().maxFrame);
	VkCommandBufferAllocateInfo commandBufferAllocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		//.pNext = ,
		.commandPool = VkCtx::Instance().commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = VkCtx::Instance().maxFrame,
	};

	vkAllocateCommandBuffers(VkCtx::Instance().device, &commandBufferAllocateInfo, swapchainCommandBuffers.data());
}

void Swapchain::CreateDepthTexture()
{
	VkCtx::Instance().depth = std::make_shared<Texture>();

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
		vkGetPhysicalDeviceFormatProperties(VkCtx::Instance().physicalDevice, formats[i], &formatProperties);

		if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			depthFormat = formats[i];
			break;
		}
	}

	VkCtx::Instance().depth->CreateImage(swapchainExtent, 1, SampleCount::SAMPLE_1BIT, (PipelineFormat)depthFormat, ImageTiling::OPTIMAL, ImageUsage::DEPTH_STENCIL, MemoryFlags::GPU).CreateImageView(ImageAspect::DEPTH | ImageAspect::STENCIL).TransitionImageLayout(ImageLayout::DEPTH_STENCIL);
}

QueueFamilyIndices Swapchain::FindQueueFamilies(VkPhysicalDevice pPhysicalDevice)
{
	unsigned queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(VkCtx::Instance().physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(VkCtx::Instance().physicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) VkCtx::Instance().queueFamilyIndices.graphicsFamily = i;

		unsigned presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(VkCtx::Instance().physicalDevice, i, VkCtx::Instance().surface, &presentSupport);

		if (presentSupport) VkCtx::Instance().queueFamilyIndices.presentFamily = i;
		if (VkCtx::Instance().queueFamilyIndices.IsComplete()) break;

		i++;
	}

	return VkCtx::Instance().queueFamilyIndices;
}

Swapchain::SwapchainSupportDetails Swapchain::QuerySwapchainSupport(VkPhysicalDevice pPhysicalDevice, VkSurfaceKHR pSurface)
{
	SwapchainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pPhysicalDevice, pSurface, &details.surfaceCapabilities);

	unsigned formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(pPhysicalDevice, pSurface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.surfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(pPhysicalDevice, pSurface, &formatCount, details.surfaceFormats.data());
	}

	unsigned presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(pPhysicalDevice, pSurface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(pPhysicalDevice, pSurface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR Swapchain::ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& pAvailableFormats)
{
	for (const auto& availableFormat : pAvailableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return availableFormat;
	}

	return pAvailableFormats[0];
}

VkPresentModeKHR Swapchain::ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& pAvailablePresentModes)
{
	for (const auto& availablePresentMode : pAvailablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) return availablePresentMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& pSurfaceCapabilities)
{
	if (pSurfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return pSurfaceCapabilities.currentExtent;
	else
	{
		unsigned width = 800, height = 600; //TODO

		VkExtent2D actualExtent
		{
			.width = width,
			.height = height
		};

		actualExtent.width = std::clamp(actualExtent.width, pSurfaceCapabilities.minImageExtent.width, pSurfaceCapabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, pSurfaceCapabilities.minImageExtent.height, pSurfaceCapabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void Swapchain::Init()
{
	CreateSwapchain();
	CreateSwapchainImageViews();
	CreateCommandBuffers();
	CreateDepthTexture();
}
