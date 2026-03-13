#include "D:/GitHub/Imagination-Engine/Refactor/Imagination - Current/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
#include "ImgnVulkan.h"
#include "HLSL.h"
using namespace vk;
using namespace Math;

static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
{
	if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
	{
		std::cout << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
	}

	return vk::False;
}

bool ImgnVulkan::IsDeviceSuitable(vk::raii::PhysicalDevice const& pPhysicalDevice)
{
	return false;
}

void ImgnVulkan::CleanupSwapchain()
{
	_swapchainImages.clear();
	_swapchain = nullptr;
}

void ImgnVulkan::RecreateSwapchain()
{
	_device.waitIdle();

	CleanupSwapchain();

	CreateSwapchain();
	CreateImageViews();
}

void ImgnVulkan::SetupDebugMessenger()
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

	_debugMessenger = _instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

void ImgnVulkan::PickPhysicalDevice()
{
	auto physicalDevices = vk::raii::PhysicalDevices(_instance);
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
	if (!candidates.empty() && candidates.rbegin()->first > 0) _physicalDevice = candidates.rbegin()->second;
	else throw std::runtime_error("failed to find a suitable GPU!");
}

void ImgnVulkan::RecordCommandBuffer(uint32_t pImageIdx)
{
	auto& commandBuffer = _commandBuffers[_frameIdx];
	commandBuffer.begin({});

	TransitionImageLayout(pImageIdx, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, {}, vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eColorAttachmentOutput);

	vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

	vk::RenderingAttachmentInfo attachmentInfo
	{
		.imageView = _swapchainImageViews[pImageIdx],
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor
	};

	vk::RenderingInfo renderingInfo
	{
		.renderArea
		{
			.offset = { 0, 0 },
			.extent = _swapchainExtent
		},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachmentInfo
	};

	commandBuffer.beginRendering(renderingInfo);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
	commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(_swapchainExtent.width), static_cast<float>(_swapchainExtent.height), 0.0f, 1.0f));
	commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapchainExtent));

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, *_descriptorSets[_frameIdx], nullptr);
	commandBuffer.bindVertexBuffers(0, *_vertexBuffer.buffer, { 0 });
	commandBuffer.bindIndexBuffer(*_indexBuffer.buffer, 0, vk::IndexType::eUint16);

	commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

	commandBuffer.endRendering();

	TransitionImageLayout(pImageIdx, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::AccessFlagBits2::eColorAttachmentWrite, {}, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe);

	commandBuffer.end();
}

uint32_t ImgnVulkan::FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps)
{
	vk::PhysicalDeviceMemoryProperties memProperties = _physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((pTypeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & pProps) == pProps)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

vk::Extent2D ImgnVulkan::ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const& pCapabilities)
{
	if (pCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return pCapabilities.currentExtent;

	return
	{
		std::clamp<uint32_t>(_win->width, pCapabilities.minImageExtent.width, pCapabilities.maxImageExtent.width),
		std::clamp<uint32_t>(_win->height, pCapabilities.minImageExtent.height, pCapabilities.maxImageExtent.height)
	};
}

uint32_t ImgnVulkan::ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& pCapabilities)
{
	auto minImageCount = std::max(3u, pCapabilities.minImageCount);
	if ((0 < pCapabilities.maxImageCount) && (pCapabilities.maxImageCount < minImageCount)) minImageCount = pCapabilities.maxImageCount;

	return minImageCount;
}

vk::SurfaceFormatKHR ImgnVulkan::ChooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& pAvailablePresentModes)
{
	const auto formatIter = std::ranges::find_if(pAvailablePresentModes, [](const auto& format)
		{
			return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
		});

	return formatIter != pAvailablePresentModes.end() ? *formatIter : pAvailablePresentModes[0];
}

std::vector<uint32_t> ImgnVulkan::GetSPV(const std::string& pShader, const std::wstring& pTarget, const std::wstring& pEntryPoint)
{
	std::vector<uint32_t> spv;

	DxcBuffer sourceBuffer;
	sourceBuffer.Ptr = pShader.c_str();
	sourceBuffer.Size = pShader.size();
	sourceBuffer.Encoding = DXC_CP_ACP;

	std::vector<LPCWSTR> args
	{
		L"-spirv",
		L"-T",
		pTarget.c_str(),
		L"-E",
		pEntryPoint.c_str(),
#ifndef NDEBUG
		L"-Zi",
		L"-Qembed_debug"
#endif // NDEBUG
	};

	ComPtr<IDxcResult> result;
	_compiler->Compile(&sourceBuffer, args.data(), static_cast<uint32_t>(args.size()), _includeHandler.Get(), IID_PPV_ARGS(&result));

	//check for compilation errors
	ComPtr<IDxcBlobUtf8> errors;
	if (SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)) && errors && errors->GetStringLength() > 0)
	{
		std::stringstream ss;
		ss << "Shader compilation errors : " << errors->GetStringPointer();
		throw std::runtime_error(ss.str());
	}

	//write compilation to spv
	ComPtr<IDxcBlob> shaderBlob;
	if (SUCCEEDED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr)))
	{
		const uint64_t byteCount = shaderBlob->GetBufferSize();

		spv.resize(byteCount * 0.25f);
		std::memcpy(spv.data(), shaderBlob->GetBufferPointer(), byteCount);
	};

	return spv;
}

void ImgnVulkan::TransitionImageLayout(uint32_t pImageIdx, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::AccessFlags2 pSrcAccessMask, vk::AccessFlags2 pDstAccessMask, vk::PipelineStageFlags2 pSrcStageMask, vk::PipelineStageFlags2 pDstStageMask)
{
	vk::ImageMemoryBarrier2 barrier
	{
		.srcStageMask = pSrcStageMask,
		.srcAccessMask = pSrcAccessMask,
		.dstStageMask = pDstStageMask,
		.dstAccessMask = pDstAccessMask,
		.oldLayout = pOldLayout,
		.newLayout = pNewLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = _swapchainImages[pImageIdx],
		.subresourceRange
		{
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	vk::DependencyInfo dependencyInfo
	{
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier
	};

	_commandBuffers[_frameIdx].pipelineBarrier2(dependencyInfo);
}

vk::raii::ShaderModule ImgnVulkan::CreateShaderModule(const std::vector<uint32_t>& pCode) const
{
	vk::ShaderModuleCreateInfo createInfo
	{
		.codeSize = pCode.size() * sizeof(uint32_t),
		.pCode = pCode.data()
	};

	vk::raii::ShaderModule shaderModule(_device, createInfo);

	return shaderModule;
}

vk::PresentModeKHR ImgnVulkan::ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& pAvailablePresentModes)
{
	assert(std::ranges::any_of(pAvailablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
	return std::ranges::any_of(pAvailablePresentModes,
		[](const vk::PresentModeKHR value)
		{
			return vk::PresentModeKHR::eMailbox == value;
		}) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
}

void ImgnVulkan::CreateDevice()
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();

	for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
	{
		if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) && _physicalDevice.getSurfaceSupportKHR(qfpIndex, *_surface))
		{
			// found a queue family that supports both graphics and present
			_queueIdx = qfpIndex;
			break;
		}
	}

	if (_queueIdx == ~0) throw std::runtime_error("Could not find a queue for graphics and present -> terminating");

	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain =
	{
		{},                               // vk::PhysicalDeviceFeatures2 (empty for now)
		{.synchronization2 = true, .dynamicRendering = true },      // Enable dynamic rendering from Vulkan 1.3
		{.extendedDynamicState = true }   // Enable extended dynamic state from the extension
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

	_device = vk::raii::Device(_physicalDevice, deviceCreateInfo);
	_queue = vk::raii::Queue(_device, _queueIdx, 0);
}

void ImgnVulkan::CreateSurface()
{
	VkSurfaceKHR surface;

	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo
	{
		.hinstance = _win->GetInstance(),
		.hwnd = _win->GetHandle()
	};

	_surface = vk::raii::SurfaceKHR(_instance, surfaceCreateInfo);
}

void ImgnVulkan::CreateInstance()
{
	constexpr ApplicationInfo applicationInfo
	{
		.pApplicationName = "Imagination",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Imagination Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = ApiVersion14
	};

	// Get the required layers
	std::vector<char const*> requiredLayers;
	if (_enableValidationLayers) requiredLayers.assign(_instanceLayers.begin(), _instanceLayers.end());

	// Check if the required layers are supported by the Vulkan implementation.
	auto layerProperties = _ctx.enumerateInstanceLayerProperties();
	auto unsupportedLayerIt = std::ranges::find_if(_instanceLayers,
		[&layerProperties](auto const& requiredLayer)
		{
			return std::ranges::none_of(layerProperties,
				[requiredLayer](auto const& layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
		});

	if (unsupportedLayerIt != _instanceLayers.end()) throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));

	// Check if the required extensions are supported by the Vulkan implementation.
	auto extensionProperties = _ctx.enumerateInstanceExtensionProperties();
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

	InstanceCreateInfo instanceCreateInfo
	{
		.pApplicationInfo = &applicationInfo,
		.enabledLayerCount = static_cast<uint32_t>(_instanceLayers.size()),
		.ppEnabledLayerNames = _instanceLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(_instanceExtensions.size()),
		.ppEnabledExtensionNames = _instanceExtensions.data()
	};

	_instance = raii::Instance(_ctx, instanceCreateInfo);
}

void ImgnVulkan::CreateSwapchain()
{
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = _physicalDevice.getSurfaceCapabilitiesKHR(*_surface);
	_swapchainExtent = ChooseSwapExtent(surfaceCapabilities);

	uint32_t minImageCount = ChooseSwapMinImageCount(surfaceCapabilities);

	std::vector<vk::SurfaceFormatKHR> availableFormats = _physicalDevice.getSurfaceFormatsKHR(*_surface);
	_swapchainSurfaceFormat = ChooseSwapSurfaceFormat(availableFormats);

	std::vector<vk::PresentModeKHR> availablePresentModes = _physicalDevice.getSurfacePresentModesKHR(*_surface);
	vk::PresentModeKHR presentMode = ChooseSwapPresentMode(availablePresentModes);

	vk::SwapchainCreateInfoKHR swapchainCreateInfo
	{
		.surface = *_surface,
		.minImageCount = minImageCount,
		.imageFormat = _swapchainSurfaceFormat.format,
		.imageColorSpace = _swapchainSurfaceFormat.colorSpace,
		.imageExtent = _swapchainExtent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = surfaceCapabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = ChooseSwapPresentMode(availablePresentModes),
		.clipped = true
	};

	_swapchain = vk::raii::SwapchainKHR(_device, swapchainCreateInfo);
	_swapchainImages = _swapchain.getImages();
}
void ImgnVulkan::CreateImageViews()
{
	assert(_swapchainImageViews.empty());

	vk::ImageViewCreateInfo imageViewCreateInfo
	{
		.viewType = ImageViewType::e2D,
		.format = _swapchainSurfaceFormat.format,
		.components
		{
			.r = ComponentSwizzle::eIdentity,
			.g = ComponentSwizzle::eIdentity,
			.b = ComponentSwizzle::eIdentity,
			.a = ComponentSwizzle::eIdentity
		},
		.subresourceRange
		{
			.aspectMask = ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	for (auto& image : _swapchainImages)
	{
		imageViewCreateInfo.image = image;
		_swapchainImageViews.emplace_back(_device, imageViewCreateInfo);
	}
}

void ImgnVulkan::CreateCommandPool()
{
	vk::CommandPoolCreateInfo poolInfo
	{
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = _queueIdx
	};

	_commandPool = vk::raii::CommandPool(_device, poolInfo);
}

void ImgnVulkan::CreateSyncObjects()
{
	assert(_presentCompleteSemaphores.empty() && _renderFinishedSemaphores.empty() && _inFlightFences.empty());

	for (size_t i = 0; i < _swapchainImages.size(); i++)
	{
		_renderFinishedSemaphores.emplace_back(_device, vk::SemaphoreCreateInfo());
	}

	for (size_t i = 0; i < MAXFRAMESINFLIGHT; i++)
	{
		_presentCompleteSemaphores.emplace_back(_device, vk::SemaphoreCreateInfo());
		_inFlightFences.emplace_back(_device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
	}
}

void ImgnVulkan::CreateIndexBuffer()
{
	vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	vkBuffer stagingBuffer = {};
	CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer);

	void* data = stagingBuffer.memory.mapMemory(0, bufferSize);
	memcpy(data, indices.data(), (size_t)bufferSize);
	stagingBuffer.memory.unmapMemory();

	CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, _indexBuffer);

	CopyBuffer(stagingBuffer.buffer, _indexBuffer.buffer, bufferSize);
}

void ImgnVulkan::CreateVertexBuffer()
{
	vk::DeviceSize bufferSize = sizeof(Vertex) * vertices.size();

	vk::BufferCreateInfo stagingInfo
	{
		.size = bufferSize,
		.usage = vk::BufferUsageFlagBits::eTransferSrc,
		.sharingMode = vk::SharingMode::eExclusive
	};

	vk::raii::Buffer stagingBuffer(_device, stagingInfo);
	vk::MemoryRequirements memRequirementsStaging = stagingBuffer.getMemoryRequirements();
	vk::MemoryAllocateInfo memoryAllocateInfoStaging
	{
		.allocationSize = memRequirementsStaging.size,
		.memoryTypeIndex = FindMemoryType(memRequirementsStaging.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
	};

	vk::raii::DeviceMemory stagingBufferMemory(_device, memoryAllocateInfoStaging);

	stagingBuffer.bindMemory(stagingBufferMemory, 0);
	void* dataStaging = stagingBufferMemory.mapMemory(0, stagingInfo.size);
	memcpy(dataStaging, vertices.data(), stagingInfo.size);
	stagingBufferMemory.unmapMemory();

	vk::BufferCreateInfo bufferInfo{ .size = bufferSize,  .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, .sharingMode = vk::SharingMode::eExclusive };
	_vertexBuffer.buffer = vk::raii::Buffer(_device, bufferInfo);

	vk::MemoryRequirements memRequirements = _vertexBuffer.buffer.getMemoryRequirements();
	vk::MemoryAllocateInfo memoryAllocateInfo
	{
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
	};

	_vertexBuffer.memory = vk::raii::DeviceMemory(_device, memoryAllocateInfo);

	_vertexBuffer.buffer.bindMemory(*_vertexBuffer.memory, 0);

	CopyBuffer(stagingBuffer, _vertexBuffer.buffer, stagingInfo.size);
}

void ImgnVulkan::CreateCommandBuffer()
{
	_commandBuffers.clear();

	vk::CommandBufferAllocateInfo allocInfo
	{
		.commandPool = _commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = MAXFRAMESINFLIGHT
	};

	_commandBuffers = vk::raii::CommandBuffers(_device, allocInfo);
}

void ImgnVulkan::CreateUniformBuffers()
{
	_uniformBuffers.clear();
	_uniformsBuffersMapped.clear();

	for (size_t i = 0; i < MAXFRAMESINFLIGHT; i++)
	{
		vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
		vkBuffer buffer = {};

		CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer);

		_uniformBuffers.emplace_back(std::move(buffer));
		_uniformsBuffersMapped.emplace_back(_uniformBuffers[i].memory.mapMemory(0, bufferSize));
	}
}

void ImgnVulkan::CreateDescriptorPool()
{
	vk::DescriptorPoolSize poolSize(vk::DescriptorType::eUniformBuffer, MAXFRAMESINFLIGHT);

	vk::DescriptorPoolCreateInfo poolInfo
	{
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = MAXFRAMESINFLIGHT,
		.poolSizeCount = 1,
		.pPoolSizes = &poolSize
	};

	_descriptorPool = vk::raii::DescriptorPool(_device, poolInfo);
}

void ImgnVulkan::CreateDescriptorSets()
{
	std::vector<vk::DescriptorSetLayout> layouts(MAXFRAMESINFLIGHT, *_descriptorSetLayout);

	vk::DescriptorSetAllocateInfo allocInfo
	{
		.descriptorPool = _descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
		.pSetLayouts = layouts.data()
	};

	_descriptorSets.clear();
	_descriptorSets = _device.allocateDescriptorSets(allocInfo);

	for (size_t i = 0; i < MAXFRAMESINFLIGHT; i++)
	{
		vk::DescriptorBufferInfo bufferInfo
		{
			.buffer = _uniformBuffers[i].buffer,
			.offset = 0,
			.range = sizeof(UniformBufferObject)
		};

		vk::WriteDescriptorSet descriptorWrite
		{
			.dstSet = _descriptorSets[i],
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.pBufferInfo = &bufferInfo
		};

		_device.updateDescriptorSets(descriptorWrite, {});
	}
}

void ImgnVulkan::CreateGraphicsPipeline()
{
	vk::raii::ShaderModule vertexSM = CreateShaderModule(GetSPV(Shaders::TriangleVertexShader, _sTarget["vert"]));
	vk::raii::ShaderModule fragmentSM = CreateShaderModule(GetSPV(Shaders::TriangleFragmentShader, _sTarget["frag"]));

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo
	{
		.stage = vk::ShaderStageFlagBits::eVertex,
		.module = vertexSM,
		.pName = "main"
	};

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo
	{
		.stage = vk::ShaderStageFlagBits::eFragment,
		.module = fragmentSM,
		.pName = "main"
	};

	std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	auto bindingDesc = Vertex::GetBindingDescription();
	auto attributeDesc = Vertex::GetAttributeDescriptions();

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo
	{
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDesc,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDesc.size()),
		.pVertexAttributeDescriptions = attributeDesc.data()
	};

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly
	{
		.topology = vk::PrimitiveTopology::eTriangleList
	};

	vk::PipelineViewportStateCreateInfo viewportState
	{
		.viewportCount = 1,
		.scissorCount = 1
	};

	vk::PipelineRasterizationStateCreateInfo rasterizer
	{
		.depthClampEnable = vk::False,
		.rasterizerDiscardEnable = vk::False,
		.polygonMode = vk::PolygonMode::eFill,
		.cullMode = vk::CullModeFlagBits::eBack,
		.frontFace = vk::FrontFace::eClockwise,
		.depthBiasEnable = vk::False,
		.depthBiasSlopeFactor = 1.0f,
		.lineWidth = 1.0f
	};

	vk::PipelineMultisampleStateCreateInfo multisampling
	{
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = vk::False
	};

	vk::PipelineColorBlendAttachmentState colorBlendAttachment
	{
		.blendEnable = vk::False,
		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	};

	vk::PipelineColorBlendStateCreateInfo colorBlending
	{
		.logicOpEnable = vk::False,
		.logicOp = vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};

	std::vector dynamicStates =
	{
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};

	vk::PipelineDynamicStateCreateInfo dynamicState
	{
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data()
	};

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo
	{
		.setLayoutCount = 1,
		.pSetLayouts = &*_descriptorSetLayout,
		.pushConstantRangeCount = 0
	};

	_pipelineLayout = vk::raii::PipelineLayout(_device, pipelineLayoutInfo);

	vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo
	{
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &_swapchainSurfaceFormat.format
	};

	vk::GraphicsPipelineCreateInfo pipelineInfo
	{
		.pNext = &pipelineRenderingCreateInfo,
		.stageCount = 2,
		.pStages = shaderStages.data(),
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		.layout = _pipelineLayout,
		.renderPass = nullptr
	};

	_pipeline = vk::raii::Pipeline(_device, nullptr, pipelineInfo);
}

void ImgnVulkan::CreateDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding uboLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr);
	vk::DescriptorSetLayoutCreateInfo layoutInfo
	{
		.bindingCount = 1,
		.pBindings = &uboLayoutBinding
	};

	_descriptorSetLayout = vk::raii::DescriptorSetLayout(_device, layoutInfo);
}

void ImgnVulkan::UpdateUniformBuffer(uint32_t pCurrImage)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo
	{
		.model = RotateG(Identity<float>(), time * Radians(90.f), vec3<float>{ 0, 0, 1 }),
		.view = Identity<float>(),//LookAtL(vec4<float>{2.f, 2.f, 2.f}, vec4<float>{0.f, 0.f, 0.f}, vec4<float>{0.f, 0.f, 1.f}),
		.proj = Identity<float>(),//Projection<float>(Radians(45.f), static_cast<float>(_swapchainExtent.width) / static_cast<float>(_swapchainExtent.height), 0.1, 10.f)
	};

	memcpy(_uniformsBuffersMapped[pCurrImage], &ubo, sizeof(UniformBufferObject));
}

void ImgnVulkan::CopyBuffer(vk::raii::Buffer& pSrc, vk::raii::Buffer& pDst, vk::DeviceSize pSize)
{
	vk::CommandBufferAllocateInfo allocInfo
	{
		.commandPool = _commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};

	vk::raii::CommandBuffer commandCopyBuffer = std::move(_device.allocateCommandBuffers(allocInfo).front());

	commandCopyBuffer.begin(vk::CommandBufferBeginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	commandCopyBuffer.copyBuffer(pSrc, pDst, vk::BufferCopy(0, 0, pSize));
	commandCopyBuffer.end();

	_queue->submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandCopyBuffer }, nullptr);
	_queue->waitIdle();
}

void ImgnVulkan::CreateBuffer(vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, vk::MemoryPropertyFlags pProps, vkBuffer& pBuffer)
{
	vk::BufferCreateInfo bufferCreateInfo
	{
		.size = pSize,
		.usage = pUsage,
		.sharingMode = vk::SharingMode::eExclusive
	};

	pBuffer.buffer = vk::raii::Buffer(_device, bufferCreateInfo);

	vk::MemoryRequirements memReqs = pBuffer.buffer.getMemoryRequirements();
	vk::MemoryAllocateInfo memoryAllocateInfo{ .allocationSize = memReqs.size, .memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) };

	pBuffer.memory = vk::raii::DeviceMemory(_device, memoryAllocateInfo);

	pBuffer.buffer.bindMemory(*pBuffer.memory, 0);
}

void ImgnVulkan::Cleanup()
{
	CleanupSwapchain();
}

void ImgnVulkan::InitVulkan(ImgnWindow* pWindow)
{
	_win = pWindow;

	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
	_utils->CreateDefaultIncludeHandler(&_includeHandler);

	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateDevice();
	CreateSwapchain();
	CreateImageViews();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateCommandPool();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffer();
	CreateSyncObjects();
}

void ImgnVulkan::DrawFrame()
{
	auto fenceResult = _device.waitForFences(*_inFlightFences[_frameIdx], vk::True, UINT64_MAX);
	if (fenceResult != vk::Result::eSuccess) throw std::runtime_error("failed to wait for fence!");

	_device.resetFences(*_inFlightFences[_frameIdx]);

	auto [result, imageIndex] = _swapchain.acquireNextImage(UINT64_MAX, *_presentCompleteSemaphores[_frameIdx], nullptr);

	if (result == vk::Result::eErrorOutOfDateKHR)
	{
		RecreateSwapchain();
		return;
	}

	_commandBuffers[_frameIdx].reset();

	RecordCommandBuffer(imageIndex);

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

	const vk::SubmitInfo submitInfo
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*_presentCompleteSemaphores[_frameIdx],
		.pWaitDstStageMask = &waitDestinationStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &*_commandBuffers[_frameIdx],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &*_renderFinishedSemaphores[imageIndex]
	};

	UpdateUniformBuffer(_frameIdx);

	_queue->submit(submitInfo, *_inFlightFences[_frameIdx]);

	const vk::PresentInfoKHR presentInfoKHR
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*_renderFinishedSemaphores[imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &*_swapchain,
		.pImageIndices = &imageIndex
	};

	result = _queue->presentKHR(presentInfoKHR);
	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || _framebufferResized)
	{
		_framebufferResized = false;
		RecreateSwapchain();
	}

	_frameIdx = (_frameIdx + 1) % MAXFRAMESINFLIGHT;
}
