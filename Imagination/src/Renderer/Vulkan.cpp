#include "pch.hpp"
#include "Vulkan.hpp"
#include "gltf/stb_image.h"
#include "EngineShaders.h"
#include "Imgn/UI/UIShader.h"

static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
{
	if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
	{
		std::cout << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
	}

	return vk::False;
}

void Vulkan::CreateDXC()
{
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
	_utils->CreateDefaultIncludeHandler(&_includeHandler);
}

void Vulkan::CreateDevice()
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = _physicalDevice->getQueueFamilyProperties();

	for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
	{
		if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) && _physicalDevice->getSurfaceSupportKHR(qfpIndex, *_surface))
		{
			// found a queue family that supports both graphics and present
			_queueIdx = qfpIndex;
			break;
		}
	}

	if (_queueIdx == ~0) throw std::runtime_error("Could not find a queue for graphics and present -> terminating");

	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceVulkan14Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceDescriptorIndexingFeatures> featureChain =
	{
		{.features = {.samplerAnisotropy = true} },                               // vk::PhysicalDeviceFeatures2 (empty for now)
		{.synchronization2 = true, .dynamicRendering = true },      // Enable dynamic rendering from Vulkan 1.3
		{.pushDescriptor = true},
		{.extendedDynamicState = true },   // Enable extended dynamic state from the extension
		{.shaderSampledImageArrayNonUniformIndexing = true, .descriptorBindingSampledImageUpdateAfterBind = true, .descriptorBindingUpdateUnusedWhilePending = true, .descriptorBindingPartiallyBound = true, .descriptorBindingVariableDescriptorCount = true, .runtimeDescriptorArray = true}
	};

	float queuePriority = 0.5f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo
	{
		.queueFamilyIndex = _queueIdx,
		.queueCount = 1, .pQueuePriorities =
		&queuePriority
	};

	vk::DeviceCreateInfo deviceCreateInfo
	{
		.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &deviceQueueCreateInfo,
		.enabledExtensionCount = static_cast<uint32_t>(_deviceExtensions.size()),
		.ppEnabledExtensionNames = _deviceExtensions.data()
	};

	_device = Unique<vk::raii::Device>(_physicalDevice->createDevice(deviceCreateInfo));
	_queue = Unique<vk::raii::Queue>(_device->getQueue(_queueIdx, 0));
}

void Vulkan::CreateSurface(void* pWindowHandle)
{
	VkSurfaceKHR surface;

	HWND hwnd = static_cast<HWND>(pWindowHandle);
	HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hwnd, GWLP_HINSTANCE));

	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo
	{
		.hinstance = hInst,
		.hwnd = hwnd
	};

	_surface = Unique<vk::raii::SurfaceKHR>(_instance->createWin32SurfaceKHR(surfaceCreateInfo));
}

void Vulkan::CreateInstance()
{
	constexpr vk::ApplicationInfo applicationInfo
	{
		.pApplicationName = "Imagination",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Imagination Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion14
	};

	// Get the required layers
	std::vector<char const*> requiredLayers;
	if (_enableValidationLayers) requiredLayers.assign(_instanceLayers.begin(), _instanceLayers.end());

	// Check if the required layers are supported by the Vulkan implementation.
	auto layerProperties = _ctx->enumerateInstanceLayerProperties();
	auto unsupportedLayerIt = std::ranges::find_if(_instanceLayers,
		[&layerProperties](auto const& requiredLayer)
		{
			return std::ranges::none_of(layerProperties,
				[requiredLayer](auto const& layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
		});

	if (unsupportedLayerIt != _instanceLayers.end()) throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));

	// Check if the required extensions are supported by the Vulkan implementation.
	auto extensionProperties = _ctx->enumerateInstanceExtensionProperties();
	auto unsupportedPropertyIt =
		std::ranges::find_if(_instanceExtensions,
			[&extensionProperties](auto const& requiredExtension)
			{
				return std::ranges::none_of(extensionProperties,
					[requiredExtension](auto const& extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
			});
	if (unsupportedPropertyIt != _instanceExtensions.end())
	{
		throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
	}

	vk::InstanceCreateInfo instanceCreateInfo
	{
		.pApplicationInfo = &applicationInfo,
		.enabledLayerCount = static_cast<uint32_t>(_instanceLayers.size()),
		.ppEnabledLayerNames = _instanceLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(_instanceExtensions.size()),
		.ppEnabledExtensionNames = _instanceExtensions.data()
	};

	_instance = Unique<vk::raii::Instance>(_ctx->createInstance(instanceCreateInfo));
}

void Vulkan::CreateSwapchain()
{
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = _physicalDevice->getSurfaceCapabilitiesKHR(*_surface);
	_swapchainExtent = ChooseSwapchainExtent(surfaceCapabilities);

	uint32_t minImageCount = ChooseSwapchainMinImageCount(surfaceCapabilities);

	std::vector<vk::SurfaceFormatKHR> availableFormats = _physicalDevice->getSurfaceFormatsKHR(*_surface);
	_swapchainSurfaceFormat = ChooseSwapchainSurfaceFormat(availableFormats);

	std::vector<vk::PresentModeKHR> availablePresentModes = _physicalDevice->getSurfacePresentModesKHR(*_surface);
	vk::PresentModeKHR presentMode = ChooseSwapchainPresentMode(availablePresentModes);

	vk::SwapchainCreateInfoKHR swapchainCreateInfo
	{
		.surface = *_surface,
		.minImageCount = minImageCount,
		.imageFormat = _swapchainSurfaceFormat.format,
		.imageColorSpace = _swapchainSurfaceFormat.colorSpace,
		.imageExtent = _swapchainExtent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = surfaceCapabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = ChooseSwapchainPresentMode(availablePresentModes),
		.clipped = true
	};

	_swapchain = Unique<vk::raii::SwapchainKHR>(_device->createSwapchainKHR(swapchainCreateInfo));
	_swapchainImages = _swapchain->getImages();

	if (_swapchainImages.empty()) { throw std::runtime_error("Swapchain returned no images"); }
}

void Vulkan::CreateSwapchainImageViews()
{
	_swapchainImageViews.clear();
	_swapchainImageViews.reserve(_swapchainImages.size());

	for (auto& image : _swapchainImages)
	{
		vk::ImageViewCreateInfo imageViewCreateInfo
		{
			.image = image,
			.viewType = vk::ImageViewType::e2D,
			.format = _swapchainSurfaceFormat.format,
			.components
			{
				.r = vk::ComponentSwizzle::eIdentity,
				.g = vk::ComponentSwizzle::eIdentity,
				.b = vk::ComponentSwizzle::eIdentity,
				.a = vk::ComponentSwizzle::eIdentity
			},
			.subresourceRange
			{
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		//CreateImageView(image, _swapchainSurfaceFormat.format, vk::ImageAspectFlagBits::eColor)

		_swapchainImageViews.emplace_back(Unique<vk::raii::ImageView>(_device->createImageView(imageViewCreateInfo)));
	}
}

void Vulkan::CreateCommandPool()
{
	vk::CommandPoolCreateInfo poolInfo
	{
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = _queueIdx
	};

	_commandPool = Unique<vk::raii::CommandPool>(_device->createCommandPool(poolInfo));
}

void Vulkan::CreateSyncObjects()
{
	_presentationReadySemaphore.clear();
	_presentationReadySemaphore.reserve(_swapchainImages.size());

	for (size_t i = 0; i < _swapchainImages.size(); i++)
	{
		_presentationReadySemaphore.push_back(Unique<vk::raii::Semaphore>(_device->createSemaphore({})));
	}

	for (size_t i = 0; i < MaxFramesInFlight; i++)
	{
		_imageAcquiredSemaphores[i] = Unique<vk::raii::Semaphore>(_device->createSemaphore({}));
		_frameFinishedFence[i] = Unique<vk::raii::Fence>(_device->createFence(vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled }));
	}
}

void Vulkan::CreateCommandBuffers()
{
	//_commandBuffers.clear();

	vk::CommandBufferAllocateInfo allocInfo
	{
		.commandPool = *_commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = MaxFramesInFlight
	};

	//_commandBuffers.resize(allocInfo.commandBufferCount);

	for (uint8_t i = 0; auto& commandBuffer : _device->allocateCommandBuffers(allocInfo))
	{
		_commandBuffers[i] = Unique<vk::raii::CommandBuffer>(std::move(commandBuffer));
		i++;
	}
}

void Vulkan::CreateDescriptorPool()
{
	vk::DescriptorPoolSize poolSize
	{
		.type = vk::DescriptorType::eCombinedImageSampler,
		.descriptorCount = NumDescriptorsStreaming
	};

	vk::DescriptorPoolCreateInfo poolInfo
	{
		.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind | vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = 1,
		.poolSizeCount = 1,
		.pPoolSizes = &poolSize
	};

	_textureDescriptorPool = Unique<vk::raii::DescriptorPool>(_device->createDescriptorPool(poolInfo));
}

void Vulkan::CreateDescriptorSets()
{
	vk::DescriptorSetLayout layout = **_textureDescriptorSetLayout;

	vk::DescriptorSetAllocateInfo allocateInfo
	{
		.descriptorPool = **_textureDescriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &layout
	};

	std::vector<vk::raii::DescriptorSet> descriptorSets = _device->allocateDescriptorSets(allocateInfo);

	_textureDescriptorSet = Unique<vk::raii::DescriptorSet>(std::move(descriptorSets.front()));
}

void Vulkan::CreateTAASampler()
{
	vk::PhysicalDeviceProperties properties = _physicalDevice->getProperties();

	vk::SamplerCreateInfo samplerInfo
	{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eClampToEdge,
		.addressModeV = vk::SamplerAddressMode::eClampToEdge,
		.addressModeW = vk::SamplerAddressMode::eClampToEdge,
		.mipLodBias = 0.f,
		.anisotropyEnable = vk::False,
		.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
		.compareEnable = vk::False,
		.compareOp = vk::CompareOp::eAlways,
		.minLod = 0.f,
		.maxLod = 0.f,
		.borderColor = vk::BorderColor::eFloatOpaqueBlack,
		.unnormalizedCoordinates = vk::False
	};

	_taaSampler = Unique<vk::raii::Sampler>(_device->createSampler(samplerInfo));
}

void Vulkan::CreateTextureSampler()
{
	vk::PhysicalDeviceProperties properties = _physicalDevice->getProperties();

	vk::SamplerCreateInfo samplerInfo
	{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.mipLodBias = 0.f,
		.anisotropyEnable = vk::True,
		.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
		.compareEnable = vk::False,
		.compareOp = vk::CompareOp::eAlways,
		.minLod = 0.f,
		.maxLod = 0.f,
		.borderColor = vk::BorderColor::eIntOpaqueBlack,
		.unnormalizedCoordinates = vk::False
	};

	_textureSampler = Unique<vk::raii::Sampler>(_device->createSampler(samplerInfo));
}

void Vulkan::RecreateSwapchain()
{
	_device->waitIdle();

	_swapchainImages.clear();
	_swapchain = nullptr;

	CreateSwapchain();
	CreateSwapchainImageViews();
}

void Vulkan::PickPhysicalDevice()
{
	auto physicalDevices = vk::raii::PhysicalDevices(*_instance);
	if (physicalDevices.empty()) throw std::runtime_error("failed to find GPUs with Vulkan support!");

	// Use an ordered map to automatically sort candidates by increasing score
	std::multimap<int, vk::raii::PhysicalDevice> candidates;

	for (const auto& physicalDevice : physicalDevices)
	{
		auto deviceProperties = physicalDevice.getProperties();
		auto deviceFeatures = physicalDevice.getFeatures();
		uint32_t score = 0;

		// Discrete GPUs have a significant performance advantage
		if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) score += 1000;

		// Maximum possible size of textures affects graphics quality
		score += deviceProperties.limits.maxImageDimension2D;

		// Application can't function without geometry shaders
		if (!deviceFeatures.geometryShader) continue;

		candidates.insert(std::make_pair(score, physicalDevice));
	}

	// Check if the best candidate is suitable at all
	if (!candidates.empty() && candidates.rbegin()->first > 0) _physicalDevice = Unique<vk::raii::PhysicalDevice>(candidates.rbegin()->second);
	else throw std::runtime_error("failed to find a suitable GPU!");
}

void Vulkan::SetupDebugMessenger()
{
	if (!_enableValidationLayers) return;

	vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
	vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT
	{
		.messageSeverity = severityFlags,
		.messageType = messageTypeFlags,
		.pfnUserCallback = &DebugCallback
	};

	_debugMessenger = Unique<vk::raii::DebugUtilsMessengerEXT>(_instance->createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT));
}

void Vulkan::RecreateSurfaceAndSwapchain()
{
	_device->waitIdle();

	// Destroy swapchain-dependent objects first.
	_swapchainImageViews.clear();
	_presentationReadySemaphore.clear();
	_swapchainImages.clear();
	_swapchain.reset();

	// The old surface is no longer valid.
	_surface.reset();

	CreateSurface(_info.windowHandle);

	if (!_physicalDevice->getSurfaceSupportKHR(_queueIdx, *_surface))
	{
		throw std::runtime_error(
			"Current queue no longer supports the recreated surface"
		);
	}

	CreateSwapchain();
	CreateSwapchainImageViews();
	CreateSyncObjects();

	_activeImageIdx = 0;
	_frameInFlightIdx = 0;
}

uint32_t Vulkan::FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps)
{
	vk::PhysicalDeviceMemoryProperties memProperties = _physicalDevice->getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((pTypeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & pProps) == pProps)
		{
			return i;
		}
	}

	IMGN_FATAL("failed to find suitable memory type!");
	throw std::runtime_error("failed to find suitable memory type!");
}

vk::Extent2D Vulkan::ChooseSwapchainExtent(vk::SurfaceCapabilitiesKHR const& pCapabilities)
{
	if (pCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return pCapabilities.currentExtent;

	return
	{
		std::clamp<uint32_t>(_info.width, pCapabilities.minImageExtent.width, pCapabilities.maxImageExtent.width),
		std::clamp<uint32_t>(_info.height, pCapabilities.minImageExtent.height, pCapabilities.maxImageExtent.height)
	};
}

void Vulkan::CopyBuffer(vk::raii::Buffer& pSrc, vk::raii::Buffer& pDst, vk::DeviceSize pSize)
{
	unique<vk::raii::CommandBuffer> commandBuffer = StartSingleTimeCommand();

	commandBuffer->copyBuffer(pSrc, pDst, vk::BufferCopy(0, 0, pSize));

	EndSingleTimeCommand(*commandBuffer);
}

uint32_t Vulkan::ChooseSwapchainMinImageCount(vk::SurfaceCapabilitiesKHR const& pCapabilities)
{
	auto minImageCount = std::max(2u, pCapabilities.minImageCount);
	if ((0 < pCapabilities.maxImageCount) && (pCapabilities.maxImageCount < minImageCount)) minImageCount = pCapabilities.maxImageCount;

	return minImageCount;
}

vk::PresentModeKHR Vulkan::ChooseSwapchainPresentMode(std::vector<vk::PresentModeKHR> const& pAvailablePresentModes)
{
	assert(std::ranges::any_of(pAvailablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
	return std::ranges::any_of(pAvailablePresentModes,
		[](const vk::PresentModeKHR value)
		{
			return vk::PresentModeKHR::eMailbox == value;
		}) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
}

vk::SurfaceFormatKHR Vulkan::ChooseSwapchainSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& pAvailableFormats)
{
	const auto formatIter = std::ranges::find_if(pAvailableFormats, [](const auto& format)
		{
			return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
		});

	return formatIter != pAvailableFormats.end() ? *formatIter : pAvailableFormats[0];
}

void Vulkan::CopyBufferToImage(uint32_t pWidth, uint32_t pHeight, const vk::raii::Buffer& pBuffer, vk::raii::Image& pImage)
{
	unique<vk::raii::CommandBuffer> commandBuffer = StartSingleTimeCommand();

	vk::BufferImageCopy region
	{
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
		.imageOffset = {0, 0, 0},
		.imageExtent = {pWidth, pHeight, 1}
	};

	commandBuffer->copyBufferToImage(pBuffer, pImage, vk::ImageLayout::eTransferDstOptimal, { region });

	EndSingleTimeCommand(*commandBuffer);
}

