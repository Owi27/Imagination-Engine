#include "pch.h"
#include "VkBackend.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#include "Window.h"

bool VkBackend::CheckCompatibility(const char** pInstanceExtensions, const char** pDeviceExtensions)
{
	const char* use_surface = VK_KHR_SURFACE_EXTENSION_NAME;
	const char* use_swapchain = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	//	const char* platform_surface = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
	const char* platform_device = nullptr;

	return false;
}

bool VkBackend::CheckLayerSupport()
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

void VkBackend::PickPhysicalDevice()
{
	unsigned deviceCount = 0;
	vkEnumeratePhysicalDevices(VkCtx::Instance().instance, &deviceCount, nullptr);

	if (deviceCount == 0) return;;

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(VkCtx::Instance().instance, &deviceCount, physicalDevices.data());

	for (const auto& device : physicalDevices)
	{
		if (IsPhysicalDeviceSuitable(device))
		{
			VkCtx::Instance().physicalDevice = device;
			break;
		}
	}
}

bool VkBackend::IsPhysicalDeviceSuitable(VkPhysicalDevice pPhysicalDevice)
{
	VkCtx::Instance().queueFamilyIndices = FindQueueFamilies(pPhysicalDevice);
	bool extensionsSupported = CheckDeviceExtensionSupport(pPhysicalDevice);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		auto swapChainSupport = _swapchain.QuerySwapchainSupport(pPhysicalDevice, VkCtx::Instance().surface);
		swapChainAdequate = !swapChainSupport.surfaceFormats.empty() && !swapChainSupport.presentModes.empty();
	}

	return VkCtx::Instance().queueFamilyIndices.IsComplete() && extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices VkBackend::FindQueueFamilies(VkPhysicalDevice pPhysicalDevice)
{
	unsigned queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(pPhysicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(pPhysicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) VkCtx::Instance().queueFamilyIndices.graphicsFamily = i;

		unsigned presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(pPhysicalDevice, i, VkCtx::Instance().surface, &presentSupport);

		if (presentSupport) VkCtx::Instance().queueFamilyIndices.presentFamily = i;
		if (VkCtx::Instance().queueFamilyIndices.IsComplete()) break;

		i++;
	}

	return VkCtx::Instance().queueFamilyIndices;
}

void VkBackend::CreateInstance(unsigned pLayerCount, const char** pLayers, unsigned pExtensionCount, const char** pExtensions)
{
	if (_enableValidationLayers && !CheckLayerSupport()) return;

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

	vkCreateInstance(&instanceCreateInfo, nullptr, &VkCtx::Instance().instance);
}

void VkBackend::CreateDevice()
{
	VkCtx::Instance().queueFamilyIndices = FindQueueFamilies(VkCtx::Instance().physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<unsigned> uniqueQueueFamilies = { VkCtx::Instance().queueFamilyIndices.graphicsFamily.value(), VkCtx::Instance().queueFamilyIndices.presentFamily.value() };

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

	VkPhysicalDeviceVulkan14Features vulkan14features
	{

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

	vkCreateDevice(VkCtx::Instance().physicalDevice, &createInfo, nullptr, &VkCtx::Instance().device);

	vkGetDeviceQueue(VkCtx::Instance().device, VkCtx::Instance().queueFamilyIndices.graphicsFamily.value(), 0, &VkCtx::Instance().graphicsQueue);
	vkGetDeviceQueue(VkCtx::Instance().device, VkCtx::Instance().queueFamilyIndices.presentFamily.value(), 0, &VkCtx::Instance().presentQueue);

	VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT,
	};

	VkPhysicalDeviceProperties2 properties2
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
		.pNext = &descriptorBufferProperties
	};

	vkGetPhysicalDeviceProperties2(VkCtx::Instance().physicalDevice, &properties2);

	VkCtx::Instance().uniformSize = descriptorBufferProperties.uniformBufferDescriptorSize;
	VkCtx::Instance().storageSize = descriptorBufferProperties.storageBufferDescriptorSize;
	VkCtx::Instance().combinedSamplerSize = descriptorBufferProperties.combinedImageSamplerDescriptorSize;
	VkCtx::Instance().setAlign = descriptorBufferProperties.descriptorBufferOffsetAlignment;

	VkCtx::Instance().vkGetDescriptorEXT = reinterpret_cast<PFN_vkGetDescriptorEXT>(vkGetDeviceProcAddr(VkCtx::Instance().device, "vkGetDescriptorEXT"));
	VkCtx::Instance().vkGetDescriptorSetLayoutSizeEXT = reinterpret_cast<PFN_vkGetDescriptorSetLayoutSizeEXT>(vkGetDeviceProcAddr(VkCtx::Instance().device, "vkGetDescriptorSetLayoutSizeEXT"));
	VkCtx::Instance().vkGetDescriptorSetLayoutBindingOffsetEXT = reinterpret_cast<PFN_vkGetDescriptorSetLayoutBindingOffsetEXT>(vkGetDeviceProcAddr(VkCtx::Instance().device, "vkGetDescriptorSetLayoutBindingOffsetEXT"));
	VkCtx::Instance().vkCmdBindDescriptorBuffersEXT = reinterpret_cast<PFN_vkCmdBindDescriptorBuffersEXT>(vkGetDeviceProcAddr(VkCtx::Instance().device, "vkCmdBindDescriptorBuffersEXT"));
	VkCtx::Instance().vkCmdSetDescriptorBufferOffsetsEXT = reinterpret_cast<PFN_vkCmdSetDescriptorBufferOffsetsEXT>(vkGetDeviceProcAddr(VkCtx::Instance().device, "vkCmdSetDescriptorBufferOffsetsEXT"));
}

void VkBackend::CreateSurface()
{
	VkWin32SurfaceCreateInfoKHR createInfo
	{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		//.pNext = , 
		//.flags = , 
		.hinstance = nullptr,
		.hwnd = (HWND)Window::GetInstance().GetWindowHandle(),
	};

	vkCreateWin32SurfaceKHR(VkCtx::Instance().instance, &createInfo, nullptr, &VkCtx::Instance().surface);
}

void VkBackend::CreateCommandPool()
{
	VkCommandPoolCreateInfo commandPoolCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		//.pNext = ,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = VkCtx::Instance().queueFamilyIndices.graphicsFamily.value(),
	};

	vkCreateCommandPool(VkCtx::Instance().device, &commandPoolCreateInfo, nullptr, &VkCtx::Instance().commandPool);
}

void VkBackend::CreateSemaphoreAndFences()
{
	VkCtx::Instance().imageAvailableSemaphores.resize(VkCtx::Instance().maxFrame);
	VkCtx::Instance().renderFinishedSemaphores.resize(VkCtx::Instance().maxFrame);
	VkCtx::Instance().renderingFences.resize(VkCtx::Instance().maxFrame);

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


	for (size_t i = 0; i < VkCtx::Instance().maxFrame; i++)
	{
		vkCreateSemaphore(VkCtx::Instance().device, &semaphoreCreateInfo, nullptr, &VkCtx::Instance().imageAvailableSemaphores[i]);
		vkCreateSemaphore(VkCtx::Instance().device, &semaphoreCreateInfo, nullptr, &VkCtx::Instance().renderFinishedSemaphores[i]);
		vkCreateFence(VkCtx::Instance().device, &fenceCreateInfo, nullptr, &VkCtx::Instance().renderingFences[i]);
	}
}

void VkBackend::CreateGraphicsPipeline()
{
}

void VkBackend::CreateDefaultSampler()
{
	VkSamplerCreateInfo samplerCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		//.pNext = ,
		//.flags = ,
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias = 0,
		//.anisotropyEnable = ,
		.maxAnisotropy = 1.f,
		//.compareEnable = ,
		//.compareOp = ,
		.minLod = 0,
		.maxLod = 1.f,
		.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
		//.unnormalizedCoordinates = ,
	};

	vkCreateSampler(VkCtx::Instance().device, &samplerCreateInfo, nullptr, &VkCtx::Instance().sampler);
}

bool VkBackend::CheckDeviceExtensionSupport(VkPhysicalDevice pPhysicalDevice)
{
	unsigned extensionCount;
	vkEnumerateDeviceExtensionProperties(pPhysicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(pPhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void VkBackend::Init(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions, unsigned dExtensionCount, const char** dExtensions)
{
	_instanceLayers.resize(_instanceLayers.size() + layerCount);
	for (size_t i = _instanceLayers.size() - layerCount; i < layerCount; i++) _instanceLayers[i] = layers[i];

	_instanceExtensions.resize(_instanceExtensions.size() + extensionCount);
	for (size_t i = _instanceExtensions.size() - extensionCount; i < extensionCount; i++) _instanceExtensions[i] = extensions[i];

	_deviceExtensions.resize(_deviceExtensions.size() + dExtensionCount);
	for (size_t i = _deviceExtensions.size() - dExtensionCount; i < dExtensionCount; i++) _deviceExtensions[i] = dExtensions[i];

	std::filesystem::create_directories("shaders/SPV");

	CreateInstance(_instanceLayers.size(), _instanceLayers.data(), _instanceExtensions.size(), _instanceExtensions.data());
	CreateSurface();
	PickPhysicalDevice();
	CreateDevice();
	CreateCommandPool();
	_swapchain.Init();
	CreateSemaphoreAndFences();

	//InitDepthTexture();
	CreateGraphicsPipeline();
	CreateDefaultSampler();
}

VkCommandBuffer VkBackend::StartFrame()
{
	//Frame is now Locked
	//_frameLocked = true;

	//Wait for Queue to be ready
	//if (m_PrevFrame != mVkCtx::Instance().currentFrame)
	vkWaitForFences(VkCtx::Instance().device, 1, &VkCtx::Instance().renderingFences[VkCtx::Instance().currentFrame], VK_TRUE, ~(static_cast<uint64_t>(0)));

	vkAcquireNextImageKHR(VkCtx::Instance().device, _swapchain.swapchain, ~(0ull), VkCtx::Instance().imageAvailableSemaphores[VkCtx::Instance().currentFrame], VK_NULL_HANDLE, &VkCtx::Instance().targetFrame);

	VkCommandBuffer commandBuffer = _swapchain.swapchainCommandBuffers[VkCtx::Instance().currentFrame];

	VkCommandBufferBeginInfo commandBufferBeginInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		//.pNext = ,
		.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
		//.pInheritanceInfo = ,
	};

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

	{
		VkImageMemoryBarrier2 acquireImageMemoryBarrier
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
			.image = _swapchain.swapchainImages[VkCtx::Instance().targetFrame]->image,
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
			.pImageMemoryBarriers = &acquireImageMemoryBarrier,
		};

		vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
	}


	VkViewport viewport
	{
		.x = 0,
		.y = 0,
		.width = (float)Window::GetInstance().GetWidth(),
		.height = (float)Window::GetInstance().GetHeight(),
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
		.extent = { Window::GetInstance().GetWidth(), Window::GetInstance().GetHeight() }
	};

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	return commandBuffer;
}

void VkBackend::EndFrame(VkCommandBuffer commandBuffer)
{
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
			.image = _swapchain.swapchainImages[VkCtx::Instance().targetFrame]->image,
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
	VkSemaphore waitSemaphores[] = { VkCtx::Instance().imageAvailableSemaphores[VkCtx::Instance().currentFrame] }, signalSemaphores[] = { VkCtx::Instance().renderFinishedSemaphores[VkCtx::Instance().targetFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkCommandBuffer commandBuffers[] = { _swapchain.swapchainCommandBuffers[VkCtx::Instance().currentFrame] };

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
	vkResetFences(VkCtx::Instance().device, 1, &VkCtx::Instance().renderingFences[VkCtx::Instance().currentFrame]);
	vkQueueSubmit(VkCtx::Instance().graphicsQueue, 1, &submitInfo, VkCtx::Instance().renderingFences[VkCtx::Instance().currentFrame]);

	VkSwapchainKHR swapchains[] = { _swapchain.swapchain };
	VkPresentInfoKHR presentInfo
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		//.pNext = ,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signalSemaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &VkCtx::Instance().targetFrame,
		.pResults = nullptr,
	};
	vkQueuePresentKHR(VkCtx::Instance().presentQueue, &presentInfo);

	if (++VkCtx::Instance().currentFrame >= VkCtx::Instance().maxFrame) VkCtx::Instance().currentFrame = 0;

	//_frameLocked = false;

}
