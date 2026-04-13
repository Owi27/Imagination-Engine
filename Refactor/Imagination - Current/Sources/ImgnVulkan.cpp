#include "pch.h"
#include "ImgnVulkan.h"
#include "HLSL.h"
//using namespace Math;

#include "gltf/stb_image.h"


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
	Device::Inst().GetDevice().waitIdle();

	CleanupSwapchain();
	CreateSwapchain();
	CreateImageViews();
	CreateDepthResources();
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

	_debugMessenger = Device::Inst().GetVkInstance().createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

vk::Format ImgnVulkan::FindDepthFormat()
{
	return FindSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint }, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

void ImgnVulkan::PickPhysicalDevice()
{
	auto physicalDevices = vk::raii::PhysicalDevices(Device::Inst().GetVkInstance());
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

	Device::Inst().SetPhysicalDevice(std::move(_physicalDevice));
}

void ImgnVulkan::RecordCommandBuffer(uint32_t pImageIdx)
{
	auto& commandBuffer = _commandBuffers[_frameIdx];
	commandBuffer.begin({});

	TransitionImageLayout(_swapchainImages[pImageIdx], vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, {}, vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::ImageAspectFlagBits::eColor);
	TransitionImageLayout(_depth.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, vk::AccessFlagBits2::eDepthStencilAttachmentWrite, vk::AccessFlagBits2::eDepthStencilAttachmentWrite, vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests, vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests, vk::ImageAspectFlagBits::eDepth);

	vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.f, 0);

	vk::RenderingAttachmentInfo attachmentInfo
	{
		.imageView = _swapchainImageViews[pImageIdx],
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor
	};

	vk::RenderingAttachmentInfo depthAttachmentInfo
	{
		.imageView = _depth.imageView,
		.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eDontCare,
		.clearValue = clearDepth
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
		.pColorAttachments = &attachmentInfo,
		.pDepthAttachment = &depthAttachmentInfo
	};

	commandBuffer.beginRendering(renderingInfo);

	////commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
	commandBuffer.setViewport(0, vk::Viewport(0.0f, 0, static_cast<float>(_swapchainExtent.width), static_cast<float>(_swapchainExtent.height), 0.0f, 1.0f));
	commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapchainExtent));

	////commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, *_descriptorSets[_frameIdx], nullptr);
	//commandBuffer.bindVertexBuffers(0, *_vertexBuffer.buffer, { 0 });
	//commandBuffer.bindIndexBuffer(*_indexBuffer.buffer, 0, vk::IndexType::eUint16);

	//commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

	commandBuffer.endRendering();

	vk::RenderingInfo guiRenderingInfo
	{
		.renderArea
		{
			.offset = { 0, 0 },
			.extent = _swapchainExtent
		},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachmentInfo,
	};

	commandBuffer.beginRendering(guiRenderingInfo);
	_gui.DrawFrame(commandBuffer);
	commandBuffer.endRendering();

	TransitionImageLayout(_swapchainImages[pImageIdx], vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::AccessFlagBits2::eColorAttachmentWrite, {}, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::ImageAspectFlagBits::eColor);

	commandBuffer.end();
}

bool ImgnVulkan::HasStencilComponent(vk::Format pFormat)
{
	return pFormat == vk::Format::eD32SfloatS8Uint || pFormat == vk::Format::eD24UnormS8Uint;
}

uint32_t ImgnVulkan::FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps)
{
	vk::PhysicalDeviceMemoryProperties memProperties = Device::Inst().GetPhysicalDevice().getMemoryProperties();

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
		std::clamp<uint32_t>(_gWinW, pCapabilities.minImageExtent.width, pCapabilities.maxImageExtent.width),
		std::clamp<uint32_t>(_gWinH, pCapabilities.minImageExtent.height, pCapabilities.maxImageExtent.height)
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
		std::cout << ss.str() << '\n';
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

vk::Format ImgnVulkan::FindSupportedFormat(const std::vector<vk::Format>& pCandidates, vk::ImageTiling pTiling, vk::FormatFeatureFlags pFeatures)
{
	for (const auto format : pCandidates)
	{
		vk::FormatProperties props = Device::Inst().GetPhysicalDevice().getFormatProperties(format);

		if (pTiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & pFeatures) == pFeatures)
		{
			return format;
		}
		if (pTiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & pFeatures) == pFeatures)
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

void ImgnVulkan::TransitionImageLayout(const vk::raii::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout)
{
	vk::raii::CommandBuffer commandBuffer = BeginSingleCommand();

	vk::ImageMemoryBarrier barrier
	{
		.oldLayout = pOldLayout,
		.newLayout = pNewLayout,
		.image = pImage,
		.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
	};

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	if (pOldLayout == vk::ImageLayout::eUndefined && pNewLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (pOldLayout == vk::ImageLayout::eTransferDstOptimal && pNewLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
	EndSingleCommand(commandBuffer);
}

void ImgnVulkan::TransitionImageLayout(const vk::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::AccessFlags2 pSrcAccessMask, vk::AccessFlags2 pDstAccessMask, vk::PipelineStageFlags2 pSrcStageMask, vk::PipelineStageFlags2 pDstStageMask, vk::ImageAspectFlags pImageAspectFlags)
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
		.image = pImage,
		.subresourceRange
		{
			.aspectMask = pImageAspectFlags,
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

void ImgnVulkan::TransitionImageLayout(const vk::raii::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::AccessFlags2 pSrcAccessMask, vk::AccessFlags2 pDstAccessMask, vk::PipelineStageFlags2 pSrcStageMask, vk::PipelineStageFlags2 pDstStageMask, vk::ImageAspectFlags pImageAspectFlags)
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
		.image = pImage,
		.subresourceRange
		{
			.aspectMask = pImageAspectFlags,
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

	vk::raii::ShaderModule shaderModule(Device::Inst().GetDevice(), createInfo);

	return shaderModule;
}

vk::raii::CommandBuffer ImgnVulkan::BeginSingleCommand()
{
	vk::CommandBufferAllocateInfo allocInfo
	{
		.commandPool = Device::Inst().GetCommandPool(),
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};

	vk::raii::CommandBuffer commandBuffer = std::move(Device::Inst().GetDevice().allocateCommandBuffers(allocInfo).front());

	vk::CommandBufferBeginInfo beginInfo
	{
		.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
	};

	commandBuffer.begin(beginInfo);

	return commandBuffer;
}

void ImgnVulkan::EndSingleCommand(vk::raii::CommandBuffer& pCommandBuffer)
{
	pCommandBuffer.end();

	vk::SubmitInfo submitInfo
	{
		.commandBufferCount = 1,
		.pCommandBuffers = &*pCommandBuffer
	};

	Device::Inst().GetQueue().submit(submitInfo, nullptr);
	Device::Inst().GetQueue().waitIdle();
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
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = Device::Inst().GetPhysicalDevice().getQueueFamilyProperties();

	for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
	{
		if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) && Device::Inst().GetPhysicalDevice().getSurfaceSupportKHR(qfpIndex, *_surface))
		{
			// found a queue family that supports both graphics and present
			_queueIdx = qfpIndex;
			break;
		}
	}

	if (_queueIdx == ~0) throw std::runtime_error("Could not find a queue for graphics and present -> terminating");

	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceDescriptorIndexingFeatures> featureChain =
	{
		{.features = {.samplerAnisotropy = true} },                               // vk::PhysicalDeviceFeatures2 (empty for now)
		{.synchronization2 = true, .dynamicRendering = true },      // Enable dynamic rendering from Vulkan 1.3
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

	_device = vk::raii::Device(Device::Inst().GetPhysicalDevice(), deviceCreateInfo);
	_queue = vk::raii::Queue(_device, _queueIdx, 0);

	Device::Inst().SetDevice(std::move(_device));
	Device::Inst().SetQueue(std::move(*_queue));
}

void ImgnVulkan::CreateSurface()
{
	VkSurfaceKHR surface;

	//vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo
	//{
	//	.hinstance = _win->GetInstance(),
	//	.hwnd = _win->GetHandle()
	//};
	
	//GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE handle;

	HWND hwnd = static_cast<HWND>(_handle.window);
	HINSTANCE* hInst = reinterpret_cast<HINSTANCE*>(GetWindowLongPtr(static_cast<HWND>(hwnd), GWLP_HINSTANCE));

	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo
	{
		.hinstance = *hInst ? *hInst : nullptr,
		.hwnd = hwnd
	};

	_surface = vk::raii::SurfaceKHR(Device::Inst().GetVkInstance(), surfaceCreateInfo);
}

void ImgnVulkan::CreateInstance()
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

	vk::InstanceCreateInfo instanceCreateInfo
	{
		.pApplicationInfo = &applicationInfo,
		.enabledLayerCount = static_cast<uint32_t>(_instanceLayers.size()),
		.ppEnabledLayerNames = _instanceLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(_instanceExtensions.size()),
		.ppEnabledExtensionNames = _instanceExtensions.data()
	};

	_instance = vk::raii::Instance(_ctx, instanceCreateInfo);
	Device::Inst().SetInstance(std::move(_instance));
}

void ImgnVulkan::CreateSwapchain()
{
	vk::SurfaceCapabilitiesKHR surfaceCapabilities;
	try
	{
		surfaceCapabilities = Device::Inst().GetPhysicalDevice().getSurfaceCapabilitiesKHR(*_surface);
	}
	catch (const vk::SurfaceLostKHRError&)
	{
		CreateSurface();
	}

	//vk::SurfaceCapabilitiesKHR surfaceCapabilities = Device::Inst().GetPhysicalDevice().getSurfaceCapabilitiesKHR(*_surface);
	_swapchainExtent = ChooseSwapExtent(surfaceCapabilities);

	uint32_t minImageCount = ChooseSwapMinImageCount(surfaceCapabilities);

	std::vector<vk::SurfaceFormatKHR> availableFormats = Device::Inst().GetPhysicalDevice().getSurfaceFormatsKHR(*_surface);
	_swapchainSurfaceFormat = ChooseSwapSurfaceFormat(availableFormats);

	std::vector<vk::PresentModeKHR> availablePresentModes = Device::Inst().GetPhysicalDevice().getSurfacePresentModesKHR(*_surface);
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

	_swapchain = vk::raii::SwapchainKHR(Device::Inst().GetDevice(), swapchainCreateInfo);
	_swapchainImages = _swapchain.getImages();
}
void ImgnVulkan::CreateImageViews()
{
	_swapchainImageViews.clear();
	_swapchainImageViews.reserve(_swapchainImages.size());

	for (auto& image : _swapchainImages)
	{
		_swapchainImageViews.emplace_back(CreateImageView(image, _swapchainSurfaceFormat.format, vk::ImageAspectFlagBits::eColor));
	}
}

void ImgnVulkan::CreateCommandPool()
{
	vk::CommandPoolCreateInfo poolInfo
	{
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = _queueIdx
	};

	_commandPool = vk::raii::CommandPool(Device::Inst().GetDevice(), poolInfo);

	Device::Inst().SetCommandPool(std::move(_commandPool));
}

void ImgnVulkan::CreateSyncObjects()
{
	assert(_presentCompleteSemaphores.empty() && _renderFinishedSemaphores.empty() && _inFlightFences.empty());

	for (size_t i = 0; i < _swapchainImages.size(); i++)
	{
		_renderFinishedSemaphores.emplace_back(Device::Inst().GetDevice(), vk::SemaphoreCreateInfo());
	}

	for (size_t i = 0; i < MAXFRAMESINFLIGHT; i++)
	{
		_presentCompleteSemaphores.emplace_back(Device::Inst().GetDevice(), vk::SemaphoreCreateInfo());
		_inFlightFences.emplace_back(Device::Inst().GetDevice(), vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
	}
}

void ImgnVulkan::CreateIndexBuffer()
{
	vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	Buffer stagingBuffer = {};
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

	vk::raii::Buffer stagingBuffer(Device::Inst().GetDevice(), stagingInfo);
	vk::MemoryRequirements memRequirementsStaging = stagingBuffer.getMemoryRequirements();
	vk::MemoryAllocateInfo memoryAllocateInfoStaging
	{
		.allocationSize = memRequirementsStaging.size,
		.memoryTypeIndex = FindMemoryType(memRequirementsStaging.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
	};

	vk::raii::DeviceMemory stagingBufferMemory(Device::Inst().GetDevice(), memoryAllocateInfoStaging);

	stagingBuffer.bindMemory(stagingBufferMemory, 0);
	void* dataStaging = stagingBufferMemory.mapMemory(0, stagingInfo.size);
	memcpy(dataStaging, vertices.data(), stagingInfo.size);
	stagingBufferMemory.unmapMemory();

	vk::BufferCreateInfo bufferInfo{ .size = bufferSize,  .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, .sharingMode = vk::SharingMode::eExclusive };
	_vertexBuffer.buffer = vk::raii::Buffer(Device::Inst().GetDevice(), bufferInfo);

	vk::MemoryRequirements memRequirements = _vertexBuffer.buffer.getMemoryRequirements();
	vk::MemoryAllocateInfo memoryAllocateInfo
	{
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
	};

	_vertexBuffer.memory = vk::raii::DeviceMemory(Device::Inst().GetDevice(), memoryAllocateInfo);

	_vertexBuffer.buffer.bindMemory(*_vertexBuffer.memory, 0);

	CopyBuffer(stagingBuffer, _vertexBuffer.buffer, stagingInfo.size);
}

void ImgnVulkan::CreateTextureImage()
{
	int width, height, channels;

	stbi_set_flip_vertically_on_load(true);
	uint8_t* pixels = stbi_load("../Textures/IKA Logo.png", &width, &height, &channels, STBI_rgb_alpha);

	vk::DeviceSize imageSize = width * height * 4;

	Buffer staging = {};
	CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);

	void* data = staging.memory.mapMemory(0, imageSize);
	memcpy(data, pixels, imageSize);
	staging.memory.unmapMemory();

	stbi_image_free(pixels);

	CreateImage(width, height, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, _texture);

	TransitionImageLayout(_texture.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	CopyBufferToImage(staging.buffer, _texture.image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	TransitionImageLayout(_texture.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}

void ImgnVulkan::CreateCommandBuffer()
{
	_commandBuffers.clear();

	vk::CommandBufferAllocateInfo allocInfo
	{
		.commandPool = Device::Inst().GetCommandPool(),
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = MAXFRAMESINFLIGHT
	};

	_commandBuffers = vk::raii::CommandBuffers(Device::Inst().GetDevice(), allocInfo);
}

void ImgnVulkan::CreateDepthResources()
{
	vk::Format depthFormat = FindDepthFormat();

	CreateImage(_swapchainExtent.width, _swapchainExtent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, _depth);
	_depth.imageView = CreateImageView(_depth.image, depthFormat, vk::ImageAspectFlagBits::eDepth);
}

void ImgnVulkan::CreateUniformBuffers()
{
	_uniformBuffers.clear();
	_uniformsBuffersMapped.clear();

	for (size_t i = 0; i < MAXFRAMESINFLIGHT; i++)
	{
		vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
		Buffer buffer = {};

		CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer);

		_uniformBuffers.emplace_back(std::move(buffer));
		_uniformsBuffersMapped.emplace_back(_uniformBuffers[i].memory.mapMemory(0, bufferSize));
	}
}

void ImgnVulkan::CreateDescriptorPool()
{
	std::array poolSize =
	{
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, _totalSets),
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, _totalSets),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, _totalSets * NumDescriptorsStreaming)
	};

	vk::DescriptorPoolCreateInfo poolInfo
	{
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
		.maxSets = _totalSets,
		.poolSizeCount = static_cast<uint32_t>(poolSize.size()),
		.pPoolSizes = poolSize.data()
	};

	_descriptorPool = vk::raii::DescriptorPool(Device::Inst().GetDevice(), poolInfo);
}

void ImgnVulkan::CreateDescriptorSets()
{
	std::vector<uint32_t> variableCounts(_totalSets, NumDescriptorsStreaming);
	std::vector<vk::DescriptorSetLayout> layouts(_totalSets, *_descriptorSetLayout);

	vk::DescriptorSetVariableDescriptorCountAllocateInfo variableInfo
	{
		.descriptorSetCount = static_cast<uint32_t>(variableCounts.size()),
		.pDescriptorCounts = variableCounts.data()
	};

	vk::DescriptorSetAllocateInfo allocInfo
	{
		.pNext = &variableInfo,
		.descriptorPool = _descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
		.pSetLayouts = layouts.data()
	};

	_descriptorSets.clear();
	_descriptorSets = Device::Inst().GetDevice().allocateDescriptorSets(allocInfo);

	for (size_t i = 0; i < MAXFRAMESINFLIGHT; i++)
	{
		UpdateDescriptorSet(i, RenderPassIdx::GBuffer, _uniformBuffers[i].buffer, sizeof(UniformBufferObject), nullptr, 0, { *_texture.imageView }, *_textureSampler);
	}
	//for (size_t i = 0; i < MAXFRAMESINFLIGHT; i++)
	//{
	//	vk::DescriptorBufferInfo bufferInfo
	//	{
	//		.buffer = _uniformBuffers[i].buffer,
	//		.offset = 0,
	//		.range = sizeof(UniformBufferObject)
	//	};

	//	vk::DescriptorImageInfo imageInfo
	//	{
	//		.sampler = *_textureSampler,
	//		.imageView = _texture.imageView,
	//		.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
	//	};

	//	std::array descriptorWrites =
	//	{
	//		vk::WriteDescriptorSet
	//		{
	//			.dstSet = _descriptorSets[DescriptorSetIndex],
	//			.dstBinding = 0,
	//			.dstArrayElement = 0,
	//			.descriptorCount = 1,
	//			.descriptorType = vk::DescriptorType::eUniformBuffer,
	//			.pBufferInfo = &bufferInfo
	//		},
	//		vk::WriteDescriptorSet
	//		{
	//			.dstSet = _descriptorSets[i],
	//			.dstBinding = 2,
	//			.dstArrayElement = 0,
	//			.descriptorCount = NumDescriptorsStreaming,
	//			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
	//			.pImageInfo = &imageInfo
	//		}
	//	};

	//	Device::Inst().GetDevice().updateDescriptorSets(descriptorWrites, {});
	//}
}

void ImgnVulkan::CreateTextureSampler()
{
	vk::PhysicalDeviceProperties properties = Device::Inst().GetPhysicalDevice().getProperties();

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

	_textureSampler = vk::raii::Sampler(Device::Inst().GetDevice(), samplerInfo);
}

void ImgnVulkan::CreateGraphicsPipelines()
{
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo
	{
		.setLayoutCount = 1,
		.pSetLayouts = &*_descriptorSetLayout,
		.pushConstantRangeCount = 0
	};

	_pipelines.pipelineLayout = vk::raii::PipelineLayout(Device::Inst().GetDevice(), pipelineLayoutInfo);

	vk::PipelineColorBlendAttachmentState colorBlendAttachment
	{
		.blendEnable = vk::False,
		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
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

	vk::GraphicsPipelineCreateInfo pipelineInfo;

	/* GBuffer*/
	{
		vk::raii::ShaderModule vertexSM = CreateShaderModule(GetSPV(Shaders::GBufferVertexShader, VertexTarget));
		vk::raii::ShaderModule fragmentSM = CreateShaderModule(GetSPV(Shaders::TriangleFragmentShader, FragmentTarget));

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

		vk::PipelineDepthStencilStateCreateInfo depthStencil
		{
			.depthTestEnable = vk::True,
			.depthWriteEnable = vk::True,
			.depthCompareOp = vk::CompareOp::eLess,
			.depthBoundsTestEnable = vk::False,
			.stencilTestEnable = vk::False
		};

		std::array blendStates = { colorBlendAttachment, colorBlendAttachment, colorBlendAttachment };

		vk::PipelineColorBlendStateCreateInfo colorBlending
		{
			.logicOpEnable = vk::False,
			.logicOp = vk::LogicOp::eCopy,
			.attachmentCount = blendStates.size(),
			.pAttachments = blendStates.data()
		};

		vk::Format depthFormat = FindDepthFormat();

		std::array colorAttachmentFormats = { vk::Format::eR16G16B16A16Sfloat, vk::Format::eR16G16B16A16Sfloat, vk::Format::eR8G8B8A8Unorm };

		vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo
		{
			.colorAttachmentCount = colorAttachmentFormats.size(),
			.pColorAttachmentFormats = colorAttachmentFormats.data(),
			.depthAttachmentFormat = depthFormat
		};

		vk::GraphicsPipelineCreateInfo gBufferPipelineInfo
		{
			.pNext = &pipelineRenderingCreateInfo,
			.stageCount = 2,
			.pStages = shaderStages.data(),
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pDepthStencilState = &depthStencil,
			.pColorBlendState = &colorBlending,
			.pDynamicState = &dynamicState,
			.layout = _pipelines.pipelineLayout,
			.renderPass = nullptr
		};

		_pipelines.gBufferPipeline = vk::raii::Pipeline(Device::Inst().GetDevice(), nullptr, gBufferPipelineInfo);

		pipelineInfo = gBufferPipelineInfo;
	}

	/* Lighting */
	{
		vk::raii::ShaderModule vertexSM = CreateShaderModule(GetSPV(Shaders::LightingVertexShader, VertexTarget));
		vk::raii::ShaderModule fragmentSM = CreateShaderModule(GetSPV(Shaders::LightingFragmentShader, FragmentTarget));

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

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo
		{
			.vertexBindingDescriptionCount = 0,
			.pVertexBindingDescriptions = nullptr,
			.vertexAttributeDescriptionCount = 0,
			.pVertexAttributeDescriptions = nullptr
		};

		vk::PipelineColorBlendStateCreateInfo colorBlending
		{
			.logicOpEnable = vk::False,
			.logicOp = vk::LogicOp::eCopy,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment
		};

		std::array colorAttachment = { vk::Format::eR8G8B8A8Unorm };
		vk::Format depthFormat = vk::Format::eD32Sfloat;

		vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo
		{
			.colorAttachmentCount = colorAttachment.size(),
			.pColorAttachmentFormats = colorAttachment.data(),
			.depthAttachmentFormat = depthFormat
		};

		pipelineInfo.pNext = &pipelineRenderingCreateInfo;
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pColorBlendState = &colorBlending;

		_pipelines.lightingPipeline = vk::raii::Pipeline(Device::Inst().GetDevice(), nullptr, pipelineInfo);
	}
}

void ImgnVulkan::CreateTextureImageView()
{
	_texture.imageView = CreateImageView(_texture.image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}

void ImgnVulkan::CreateDescriptorSetLayout()
{
	std::array bindings =
	{
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, nullptr),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, nullptr),
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eCombinedImageSampler, NumDescriptorsStreaming, vk::ShaderStageFlagBits::eFragment, nullptr)
	};

	std::array<vk::DescriptorBindingFlags, 3> flags =
	{
		vk::DescriptorBindingFlags{}, vk::DescriptorBindingFlags{},
		vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending
	};

	vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlags
	{
		.bindingCount = static_cast<uint32_t>(flags.size()),
		.pBindingFlags = flags.data(),
	};

	vk::DescriptorSetLayoutCreateInfo layoutInfo
	{
		.pNext = &bindingFlags,
		.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data()
	};

	_descriptorSetLayout = vk::raii::DescriptorSetLayout(Device::Inst().GetDevice(), layoutInfo);
}

void ImgnVulkan::SetupDeferredRenderer()
{
	uint32_t width = _swapchainExtent.width, height = _swapchainExtent.height;

	_sponza = _manager.Load<Mesh>("Sponza");
	//auto gBufferVertex = _manager.Load<Shader>("GBufferVertex", Shaders::);
	//auto gBufferFrag = _manager.Load<Shader>("GBufferFragment", Shaders::);

	//gbuffer image creation
	CreateImage(width, height, vk::Format::eR16G16B16A16Sfloat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, _gBufferImages[0]);
	_gBufferImages[0].imageView = CreateImageView(_gBufferImages[0].image, vk::Format::eR16G16B16A16Sfloat, vk::ImageAspectFlagBits::eColor);

	CreateImage(width, height, vk::Format::eR16G16B16A16Sfloat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, _gBufferImages[1]);
	_gBufferImages[1].imageView = CreateImageView(_gBufferImages[1].image, vk::Format::eR16G16B16A16Sfloat, vk::ImageAspectFlagBits::eColor);

	CreateImage(width, height, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, _gBufferImages[2]);
	_gBufferImages[2].imageView = CreateImageView(_gBufferImages[2].image, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor);

	CreateImage(width, height, vk::Format::eR16G16B16A16Sfloat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, _finalImage);
	_finalImage.imageView = CreateImageView(_finalImage.image, vk::Format::eR16G16B16A16Sfloat, vk::ImageAspectFlagBits::eColor);


	_graph.AddResource("GBuffer-Position", vk::Format::eR16G16B16A16Sfloat, { width, height }, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);
	_graph.AddResource("GBuffer-Normal", vk::Format::eR16G16B16A16Sfloat, { width, height }, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);
	_graph.AddResource("GBuffer-Albedo", vk::Format::eR8G8B8A8Unorm, { width, height }, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);
	_graph.AddResource("Depth", vk::Format::eD32Sfloat, { width, height }, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageAspectFlagBits::eDepth);
	_graph.AddResource("FinalColor", vk::Format::eR8G8B8A8Unorm, { width, height }, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal, vk::ImageAspectFlagBits::eColor);

	RenderPass gBufferPass
	{
		.name = "GeometryPass",
		.inputs = {},
		.outputs = { "GBuffer-Position", "GBuffer-Normal", "GBuffer-Albedo", "Depth" },
		.descriptorSetLayout = _descriptorSetLayout,
		.Execute = [&](vk::raii::CommandBuffer& commandBuffer)
		{
			std::array<vk::RenderingAttachmentInfo, 3> colorAttachments;
			vk::RenderingAttachmentInfoKHR depthAttachment;
			vk::RenderingInfoKHR renderingInfo;

			colorAttachments[0].setImageView(_gBufferImages[0].imageView).setImageLayout(vk::ImageLayout::eColorAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore);
			colorAttachments[1].setImageView(_gBufferImages[1].imageView).setImageLayout(vk::ImageLayout::eColorAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore);
			colorAttachments[2].setImageView(_gBufferImages[2].imageView).setImageLayout(vk::ImageLayout::eColorAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore);

			depthAttachment.setImageView(_depth.imageView).setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setClearValue(vk::ClearDepthStencilValue{ 1.0f, 0 });

			renderingInfo.setRenderArea({ {0, 0}, {_swapchainExtent.width, _swapchainExtent.height} }).setLayerCount(1).setColorAttachmentCount(colorAttachments.size()).setPColorAttachments(colorAttachments.data()).setPDepthAttachment(&depthAttachment);

			commandBuffer.beginRendering(renderingInfo);

			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipelines.gBufferPipeline);
			commandBuffer.setViewport(0, vk::Viewport(0.0f, 0, static_cast<float>(_swapchainExtent.width), static_cast<float>(_swapchainExtent.height), 0.0f, 1.0f));
			commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapchainExtent));


			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelines.pipelineLayout, 0, *_descriptorSets[DescriptorSetIndex(_frameIdx, RenderPassIdx::GBuffer)], nullptr);
			commandBuffer.bindVertexBuffers(0, _sponza->GetVertexBuffer(), {0});
			commandBuffer.bindIndexBuffer(_sponza->GetIndexBuffer(), 0, vk::IndexType::eUint32);

			commandBuffer.drawIndexed(_sponza->GetIndexCount(), 1, 0, 0, 0);

			commandBuffer.endRendering();
		}
	};

	_graph.AddPass(gBufferPass);

	_graph.AddPass("GeometryPass", {}, { "GBuffer-Position", "GBuffer-Normal", "GBuffer-Albedo", "Depth" }, [&](vk::raii::CommandBuffer& commandBuffer)
		{
			std::array<vk::RenderingAttachmentInfo, 3> colorAttachments;
			vk::RenderingAttachmentInfoKHR depthAttachment;
			vk::RenderingInfoKHR renderingInfo;

			colorAttachments[0].setImageView(_gBufferImages[0].imageView).setImageLayout(vk::ImageLayout::eColorAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore);
			colorAttachments[1].setImageView(_gBufferImages[1].imageView).setImageLayout(vk::ImageLayout::eColorAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore);
			colorAttachments[2].setImageView(_gBufferImages[2].imageView).setImageLayout(vk::ImageLayout::eColorAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore);

			depthAttachment.setImageView(_depth.imageView).setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setClearValue(vk::ClearDepthStencilValue{ 1.0f, 0 });

			renderingInfo.setRenderArea({ {0, 0}, {_swapchainExtent.width, _swapchainExtent.height} }).setLayerCount(1).setColorAttachmentCount(colorAttachments.size()).setPColorAttachments(colorAttachments.data()).setPDepthAttachment(&depthAttachment);

			commandBuffer.begin({});
			commandBuffer.beginRendering(renderingInfo);

			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipelines.gBufferPipeline);
			commandBuffer.setViewport(0, vk::Viewport(0.0f, 0, static_cast<float>(_swapchainExtent.width), static_cast<float>(_swapchainExtent.height), 0.0f, 1.0f));
			commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapchainExtent));


			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelines.pipelineLayout, 0, *_descriptorSets[DescriptorSetIndex(_frameIdx, RenderPassIdx::GBuffer)], nullptr);
			commandBuffer.bindVertexBuffers(0, _sponza->GetVertexBuffer(), { 0 });
			commandBuffer.bindIndexBuffer(_sponza->GetIndexBuffer(), 0, vk::IndexType::eUint32);

			commandBuffer.drawIndexed(_sponza->GetIndexCount(), 1, 0, 0, 0);

			commandBuffer.endRendering();
			commandBuffer.end();
		});

	//_graph.AddPass("LightingPass", { "GBuffer-Position", "GBuffer-Normal", "GBuffer-Albedo", "Depth" }, {"FinalColor"}, [&](vk::raii::CommandBuffer& commandBuffer)
	//	{
	//		vk::RenderingAttachmentInfo colorAttachment;
	//		vk::RenderingInfoKHR renderingInfo;

	//		colorAttachment.setImageView(_gBufferImages[0].imageView).setImageLayout(vk::ImageLayout::eColorAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore);

	//		renderingInfo.setRenderArea({ {0, 0}, {_swapchainExtent.width, _swapchainExtent.height} }).setLayerCount(1).setColorAttachmentCount(1).setPColorAttachments(&colorAttachment);

	//		commandBuffer.beginRendering(renderingInfo);

	//		//commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
	//		//commandBuffer.setViewport(0, vk::Viewport(0.0f, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f));
	//		//commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapchainExtent));

	//		//commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, *_descriptorSets[_frameIdx], nullptr);
	//		//commandBuffer.bindVertexBuffers(0, sponza->GetVertexBuffer(), {0});
	//		//commandBuffer.bindIndexBuffer(sponza->GetIndexBuffer(), 0, vk::IndexType::eUint16);

	//		//commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

	//		commandBuffer.endRendering();
	//	});

	_graph.Compile();
}

void ImgnVulkan::UpdateUniformBuffer(uint32_t pCurrImage)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	//UniformBufferObject ubo
	//{
	//	.model = RotateG(Identity<float>(), time * Radians(90.f), vec3<float>{ 0, 0, 1 }),
	//	.view = LookAtL(vec4<float>{2.f, 2.f, 2.f}, vec4<float>{0.f, 0.f, 0.f}, vec4<float>{0.f, 0.f, 1.f}),
	//	.proj = Projection<float>(Radians(45.f), static_cast<float>(_swapchainExtent.width) / static_cast<float>(_swapchainExtent.height), 0.1, 10.f)
	//};

	//memcpy(_uniformsBuffersMapped[pCurrImage], &ubo, sizeof(UniformBufferObject));
}

vk::raii::ImageView ImgnVulkan::CreateImageView(vk::Image& pImage, vk::Format pFormat, vk::ImageAspectFlags pAspectFlags)
{
	vk::ImageViewCreateInfo imageViewCreateInfo
	{
		.image = pImage,
		.viewType = vk::ImageViewType::e2D,
		.format = pFormat,
		.components
		{
			.r = vk::ComponentSwizzle::eIdentity,
			.g = vk::ComponentSwizzle::eIdentity,
			.b = vk::ComponentSwizzle::eIdentity,
			.a = vk::ComponentSwizzle::eIdentity
		},
		.subresourceRange
		{
			.aspectMask = pAspectFlags,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	return vk::raii::ImageView(Device::Inst().GetDevice(), imageViewCreateInfo);
}

vk::raii::ImageView ImgnVulkan::CreateImageView(vk::raii::Image& pImage, vk::Format pFormat, vk::ImageAspectFlags pAspectFlags)
{
	vk::ImageViewCreateInfo imageViewCreateInfo
	{
		.image = pImage,
		.viewType = vk::ImageViewType::e2D,
		.format = pFormat,
		.components
		{
			.r = vk::ComponentSwizzle::eIdentity,
			.g = vk::ComponentSwizzle::eIdentity,
			.b = vk::ComponentSwizzle::eIdentity,
			.a = vk::ComponentSwizzle::eIdentity
		},
		.subresourceRange
		{
			.aspectMask = pAspectFlags,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	return vk::raii::ImageView(Device::Inst().GetDevice(), imageViewCreateInfo);
}

void ImgnVulkan::CopyBuffer(vk::raii::Buffer& pSrc, vk::raii::Buffer& pDst, vk::DeviceSize pSize)
{
	vk::raii::CommandBuffer commandCopyBuffer = BeginSingleCommand();

	commandCopyBuffer.copyBuffer(pSrc, pDst, vk::BufferCopy(0, 0, pSize));

	EndSingleCommand(commandCopyBuffer);
}

void ImgnVulkan::CopyBufferToImage(const vk::raii::Buffer& pBuffer, vk::raii::Image& pImage, uint32_t pWidth, uint32_t pHeight)
{
	vk::raii::CommandBuffer commandBuffer = BeginSingleCommand();

	vk::BufferImageCopy region
	{
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
		.imageOffset = {0, 0, 0},
		.imageExtent = {pWidth, pHeight, 1}
	};

	commandBuffer.copyBufferToImage(pBuffer, pImage, vk::ImageLayout::eTransferDstOptimal, { region });

	EndSingleCommand(commandBuffer);
}

void ImgnVulkan::CreateBuffer(vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Buffer& pBuffer)
{
	vk::BufferCreateInfo bufferCreateInfo
	{
		.size = pSize,
		.usage = pUsage,
		.sharingMode = vk::SharingMode::eExclusive
	};

	pBuffer.buffer = vk::raii::Buffer(Device::Inst().GetDevice(), bufferCreateInfo);

	vk::MemoryRequirements memReqs = pBuffer.buffer.getMemoryRequirements();
	vk::MemoryAllocateInfo memoryAllocateInfo{ .allocationSize = memReqs.size, .memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, pProps) };

	pBuffer.memory = vk::raii::DeviceMemory(Device::Inst().GetDevice(), memoryAllocateInfo);

	pBuffer.buffer.bindMemory(*pBuffer.memory, 0);
}

void ImgnVulkan::CreateImage(uint32_t pWidth, uint32_t pHeight, vk::Format pFormat, vk::ImageTiling pTiling, vk::ImageUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Image& pImage)
{
	vk::ImageCreateInfo imageCreateInfo
	{
		.imageType = vk::ImageType::e2D,
		.format = pFormat,
		.extent = {pWidth , pHeight, 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = vk::SampleCountFlagBits::e1,
		.tiling = pTiling,
		.usage = pUsage,
		.sharingMode = vk::SharingMode::eExclusive
	};

	pImage.image = vk::raii::Image(Device::Inst().GetDevice(), imageCreateInfo);

	vk::MemoryRequirements memRequirements = pImage.image.getMemoryRequirements();
	vk::MemoryAllocateInfo allocInfo
	{
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, pProps)
	};

	pImage.memory = vk::raii::DeviceMemory(Device::Inst().GetDevice(), allocInfo);
	pImage.image.bindMemory(pImage.memory, 0);
}

void ImgnVulkan::UpdateDescriptorSet(uint32_t pFrameIdx, RenderPassIdx pIdx, vk::Buffer pUniformBuffer, vk::DeviceSize pUniformBufferSize, vk::Buffer pStorageBuffer, vk::DeviceSize pStorageBufferSize, const std::vector<vk::ImageView>& pTextures, vk::Sampler pSampler)
{
	vk::DescriptorBufferInfo uniformBufferInfo
	{
		.buffer = pUniformBuffer,
		.offset = 0,
		.range = pUniformBufferSize
	};

	vk::DescriptorBufferInfo storageBufferInfo
	{
		.buffer = pStorageBuffer,
		.offset = 0,
		.range = pStorageBufferSize
	};

	std::vector<vk::DescriptorImageInfo> imageInfos;
	imageInfos.reserve(pTextures.size());

	for (auto& texture : pTextures)
	{
		imageInfos.push_back(vk::DescriptorImageInfo
			{
				.sampler = pSampler,
				.imageView = texture,
				.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
			});
	}

	std::vector<vk::WriteDescriptorSet> writes;

	int writeSize = 0;
	if (pUniformBuffer) writeSize++;
	if (pStorageBuffer) writeSize++;
	if (pTextures.size() > 0) writeSize++;

	writes.resize(writeSize);

	if (pUniformBuffer)
	{
		writes[--writeSize] = vk::WriteDescriptorSet
		{
			.dstSet = _descriptorSets[DescriptorSetIndex(pFrameIdx, pIdx)],
			.dstBinding = 2,
			.dstArrayElement = 0,
			.descriptorCount = static_cast<uint32_t>(imageInfos.size()),
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.pImageInfo = imageInfos.data()
		};
	}

	if (pStorageBuffer)
	{
		writes[--writeSize] = vk::WriteDescriptorSet
		{
			.dstSet = _descriptorSets[DescriptorSetIndex(pFrameIdx, pIdx)],
			.dstBinding = 1,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &storageBufferInfo
		};
	}

	if (pTextures.size() > 0)
	{
		writes[--writeSize] = vk::WriteDescriptorSet
		{
			.dstSet = _descriptorSets[DescriptorSetIndex(pFrameIdx, pIdx)],
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.pBufferInfo = &uniformBufferInfo
		};
	}

	//std::array writes =
	//{
	//	pUniformBuffer ? ,
	//	,
	//	
	//};

	Device::Inst().GetDevice().updateDescriptorSets(writes, {});
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

	Device::Inst().InitDXC();

	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateDevice();
	CreateSwapchain();
	CreateImageViews();
	CreateDescriptorSetLayout();
	CreateGraphicsPipelines();
	CreateCommandPool();
	CreateDepthResources();
	CreateTextureImage();
	CreateTextureImageView();
	CreateTextureSampler();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffer();
	CreateSyncObjects();

	SetupDeferredRenderer();

	_gui.Init(*pWindow, pWindow->width, pWindow->height);
}

void ImgnVulkan::InitVulkan(GWindow* pWindow)
{
	_gWin = pWindow;
	_gWin->GetClientWidth(_gWinW);
	_gWin->GetClientHeight(_gWinH);
	_gWin->GetWindowHandle(_handle);

	_responder.Create([&](const GEvent& e)
		{
			GWindow::Events event;
			GWindow::EVENT_DATA data;

			if (+e.Read(event, data))
			{
				switch (event)
				{
				case GWindow::Events::RESIZE:
					{
						WindowResizedEvent wre(data.clientWidth, data.clientHeight);
						EventDispatcher dispatcher(wre);
						dispatcher.Dispatch<WindowResizedEvent>([&](WindowResizedEvent& e)
							{
								RecreateSwapchain();

								return true;
							});
					}
					break;
				case GWindow::Events::DISPLAY_CLOSED:
					{
						WindowClosedEvent wce;
						EventDispatcher dispatcher(wce);
						dispatcher.Dispatch<WindowClosedEvent>([&](WindowClosedEvent& e)
							{
								Device::Inst().GetDevice().waitIdle();

								return true;
							});

					}
					break;
				}
			}
		});

	_gWin->Register(_responder);

	
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
	_utils->CreateDefaultIncludeHandler(&_includeHandler);

	Device::Inst().InitDXC();

	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateDevice();
	CreateSwapchain();
	CreateImageViews();
	CreateDescriptorSetLayout();
	CreateGraphicsPipelines();
	CreateCommandPool();
	CreateDepthResources();
	CreateTextureImage();
	CreateTextureImageView();
	CreateTextureSampler();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffer();
	CreateSyncObjects();

	SetupDeferredRenderer();

	GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE handle;
	_gWin->GetWindowHandle(handle);

	//_gWin->Register()


	_gui.Init(static_cast<HWND>(handle.window), _gWinW, _gWinH);
	//ImgnInput input(*_gWin);
	_input = new ImgnInput(*_gWin);
	uint32_t o;
	_input->bufferedInput.Observers(o);

	_gui.SetInput(&_input->bufferedInput);

	_input->bufferedInput.Observers(o);
}

void ImgnVulkan::DrawFrame()
{
	auto fenceResult = Device::Inst().GetDevice().waitForFences(*_inFlightFences[_frameIdx], vk::True, UINT64_MAX);
	if (fenceResult != vk::Result::eSuccess) throw std::runtime_error("failed to wait for fence!");

	Device::Inst().GetDevice().resetFences(*_inFlightFences[_frameIdx]);

	auto [result, imageIndex] = _swapchain.acquireNextImage(UINT64_MAX, *_presentCompleteSemaphores[_frameIdx], nullptr);

	if (result == vk::Result::eErrorOutOfDateKHR)
	{
		RecreateSwapchain();
		return;
	}

	_commandBuffers[_frameIdx].reset();
	//_commandBuffers[_frameIdx].begin({});
	//UpdateUniformBuffer(_frameIdx);

	//_graph.Execute(_commandBuffers[_frameIdx], *Device::Inst().GetQueue());
	//_gui.DrawFrame(_commandBuffers[_frameIdx]);
	RecordCommandBuffer(imageIndex);
	//_commandBuffers[_frameIdx].end();

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

	Device::Inst().GetQueue().submit(submitInfo, *_inFlightFences[_frameIdx]);

	const vk::PresentInfoKHR presentInfoKHR
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*_renderFinishedSemaphores[imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &*_swapchain,
		.pImageIndices = &imageIndex
	};

	//result = Device::Inst().GetQueue().presentKHR(presentInfoKHR);

	try
	{
		result = Device::Inst().GetQueue().presentKHR(presentInfoKHR);

		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || _framebufferResized)
		{
			_framebufferResized = false;
			RecreateSwapchain();
		}
	}
	catch (const vk::OutOfDateKHRError&)
	{
		_framebufferResized = false;
		RecreateSwapchain();
		return;
	}

	//if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || _framebufferResized)
	//{
	//	_framebufferResized = false;
	//	RecreateSwapchain();
	//}

	_frameIdx = (_frameIdx + 1) % MAXFRAMESINFLIGHT;
}

