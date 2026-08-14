#include "pch.hpp"
#include "Vulkan.hpp"
#include "gltf/stb_image.h"
#include "EngineShaders.h"

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

void Vulkan::CreateGraphicsPipelines()
{
	auto CreateSPV = [this](const std::string& pShader, const std::wstring& pTarget, const std::wstring& pEntryPoint = L"main") -> std::vector<uint32_t>
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
				L"-fspv-debug=vulkan-with-source"
		#endif // NDEBUG
			};

			ComPtr<IDxcResult> result;
			//_compiler->Compile(&sourceBuffer, args.data(), static_cast<uint32_t>(args.size()), _includeHandler.Get(), IID_PPV_ARGS(&result));

			HRESULT compileResult = _compiler->Compile(
				&sourceBuffer,
				args.data(),
				static_cast<uint32_t>(args.size()),
				_includeHandler.Get(),
				IID_PPV_ARGS(&result)
			);

			if (FAILED(compileResult) || !result)
			{
				IMGN_FATAL(
					"DXC invocation failed: 0x{:08X}",
					static_cast<uint32_t>(compileResult)
				);

				return {};
			}
			//check for compilation errors
			//ComPtr<IDxcBlobUtf8> errors;
			//if (SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)) && errors && errors->GetStringLength() > 0)
			//{
			//	std::stringstream ss;
			//	ss << "Shader compilation errors : " << errors->GetStringPointer();
			//	IMGN_FATAL("Shader compilation errors : {}", errors->GetStringPointer());
			//	throw std::runtime_error(ss.str());
			//}
			HRESULT shaderStatus = E_FAIL;
			result->GetStatus(&shaderStatus);

			ComPtr<IDxcBlobUtf8> diagnostics;
			result->GetOutput(
				DXC_OUT_ERRORS,
				IID_PPV_ARGS(&diagnostics),
				nullptr
			);

			if (diagnostics && diagnostics->GetStringLength() > 0)
			{
				IMGN_WARN("DXC diagnostics: {}", diagnostics->GetStringPointer());
			}

			if (FAILED(shaderStatus))
			{
				throw std::runtime_error("DXC shader compilation failed");
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
		};

	auto CreateShaderModule = [this](const std::vector<uint32_t>& pCode) -> vk::raii::ShaderModule
		{
			vk::ShaderModuleCreateInfo createInfo
			{
				.codeSize = pCode.size() * sizeof(uint32_t),
				.pCode = pCode.data()
			};

			vk::raii::ShaderModule shaderModule(*_device, createInfo);

			return shaderModule;
		};

	vk::PushConstantRange pcr
	{
		.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eCompute,
		.offset = 0,
		.size = 128
	};

	std::array setLayouts = { **_pushDescriptorSetLayout, **_textureDescriptorSetLayout };

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo
	{
		.setLayoutCount = setLayouts.size(),
		.pSetLayouts = setLayouts.data(),
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pcr
	};

	_pipelines.pipelineLayout = Unique<vk::raii::PipelineLayout>(_device->createPipelineLayout(pipelineLayoutInfo));

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
		vk::raii::ShaderModule vertexSM = CreateShaderModule(CreateSPV(Shaders::GBufferVertexShader, VertexTarget));
		vk::raii::ShaderModule fragmentSM = CreateShaderModule(CreateSPV(Shaders::GBufferFragmentShader, FragmentTarget));

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

		std::array shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

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
			.cullMode = vk::CullModeFlagBits::eNone,
			.frontFace = vk::FrontFace::eCounterClockwise,
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
			.depthCompareOp = vk::CompareOp::eGreater,
			.depthBoundsTestEnable = vk::False,
			.stencilTestEnable = vk::False
		};

		std::array blendStates = { colorBlendAttachment, colorBlendAttachment, colorBlendAttachment, colorBlendAttachment };

		vk::PipelineColorBlendStateCreateInfo colorBlending
		{
			.logicOpEnable = vk::False,
			.logicOp = vk::LogicOp::eCopy,
			.attachmentCount = blendStates.size(),
			.pAttachments = blendStates.data()
		};

		std::array colorAttachmentFormats =
		{
			vk::Format::eR8G8B8A8Srgb,
			vk::Format::eR8G8B8A8Unorm,
			vk::Format::eR8G8B8A8Unorm,
			vk::Format::eR8G8B8A8Srgb,
		};

		vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo
		{
			.colorAttachmentCount = colorAttachmentFormats.size(),
			.pColorAttachmentFormats = colorAttachmentFormats.data(),
			.depthAttachmentFormat = vk::Format::eD32Sfloat
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
			.layout = *_pipelines.pipelineLayout,
			.renderPass = nullptr
		};

		_pipelines.gBufferPipeline = Unique<vk::raii::Pipeline>(_device->createGraphicsPipeline(nullptr, gBufferPipelineInfo));

		pipelineInfo = gBufferPipelineInfo;
	}

	/* Lighting */
	{
		vk::raii::ShaderModule computeSM = CreateShaderModule(CreateSPV(Shaders::LightingComputeShader, ComputeTarget));

		vk::PipelineShaderStageCreateInfo computeShaderStageInfo
		{
			.stage = vk::ShaderStageFlagBits::eCompute,
			.module = computeSM,
			.pName = "main"
		};

		vk::ComputePipelineCreateInfo computePipelineCreateInfo
		{
			.stage = computeShaderStageInfo,
			.layout = *_pipelines.pipelineLayout
		};

		_pipelines.lightingPipeline = Unique<vk::raii::Pipeline>(_device->createComputePipeline(nullptr, computePipelineCreateInfo));
	}
}

//void Vulkan::CreateDescriptorPool()
//{
//	/*std::array poolSize =
//	{
//		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, _totalSets),
//		vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, _totalSets),
//		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, _totalSets * NumDescriptorsStreaming)
//	};
//
//	vk::DescriptorPoolCreateInfo poolInfo
//	{
//		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
//		.maxSets = _totalSets,
//		.poolSizeCount = static_cast<uint32_t>(poolSize.size()),
//		.pPoolSizes = poolSize.data()
//	};
//
//	_descriptorPool = vk::raii::DescriptorPool(Device::Inst().GetDevice(), poolInfo);*/
//}

void Vulkan::CreateDescriptorSetLayout()
{
	std::array bindings =
	{
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eCompute, nullptr),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eCompute, nullptr),
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eSampledImage, 10, vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eCompute, nullptr),
		vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute, nullptr)
	};

	//std::array<vk::DescriptorBindingFlags, 3> flags =
	//{
	//	vk::DescriptorBindingFlags{}, vk::DescriptorBindingFlags{},
	//	vk::DescriptorBindingFlagBits::ePartiallyBound
	//};

	//vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlags
	//{
	//	.bindingCount = static_cast<uint32_t>(flags.size()),
	//	.pBindingFlags = flags.data(),
	//};

	vk::DescriptorSetLayoutCreateInfo layoutInfo
	{
		.flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptor,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data()
	};

	_pushDescriptorSetLayout = Unique<vk::raii::DescriptorSetLayout>(_device->createDescriptorSetLayout(layoutInfo));

	//texure
	vk::DescriptorSetLayoutBinding imageBinding
	{
		.binding = 0,
		.descriptorType = vk::DescriptorType::eCombinedImageSampler,
		.descriptorCount = NumDescriptorsStreaming,
		.stageFlags = vk::ShaderStageFlagBits::eFragment,
		.pImmutableSamplers = nullptr
	};

	vk::DescriptorBindingFlags imageBindingFlags = vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind;

	vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo
	{
		.bindingCount = 1,
		.pBindingFlags = &imageBindingFlags
	};

	vk::DescriptorSetLayoutCreateInfo textureLayoutInfo
	{
		.pNext = &bindingFlagsInfo,
		.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
		.bindingCount = 1,
		.pBindings = &imageBinding
	};

	_textureDescriptorSetLayout = Unique<vk::raii::DescriptorSetLayout>(_device->createDescriptorSetLayout(textureLayoutInfo));

	//vk::raii::CommandBuffer commandBuffer;

	//vk::DescriptorBufferInfo bufferInfo
	//{
	//	.buffer = 
	//}
	//std::array writes =
	//{
	//	vk::WriteDescriptorSet
	//	{
	//		.dstSet = 0,
	//		.dstBinding = 0,
	//		.descriptorCount = 1,
	//		.descriptorType = vk::DescriptorType::eUniformBuffer,
	//		.pBufferInfo = 
	//	}
	//}
	//commandBuffer.pushDescriptorSet(vk::PipelineBindPoint::eGraphics, *_pipelines.pipelineLayout, 0, )
}

void Vulkan::CreateImageView(vk::Format pFormat, vk::ImageAspectFlags pAspectFlags, Image& pImage)
{
	vk::ImageViewCreateInfo imageViewCreateInfo
	{
		.image = *pImage.image,
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

	pImage.view = Unique<vk::raii::ImageView>(_device->createImageView(imageViewCreateInfo));
}

void Vulkan::CreateBuffer(vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Buffer& pBuffer)
{
	if (pSize == 0)
	{
		throw std::invalid_argument(
			"Vulkan::CreateBuffer called with pSize == 0"
		);
	}

	vk::BufferCreateInfo bufferCreateInfo
	{
		.size = pSize,
		.usage = pUsage,
		.sharingMode = vk::SharingMode::eExclusive
	};

	pBuffer.buffer = Unique<vk::raii::Buffer>(_device->createBuffer(bufferCreateInfo));

	vk::MemoryRequirements memReqs = pBuffer.buffer->getMemoryRequirements();
	vk::MemoryAllocateInfo memoryAllocateInfo
	{
		.allocationSize = memReqs.size,
		.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, pProps)
	};

	pBuffer.memory = Unique<vk::raii::DeviceMemory>(_device->allocateMemory(memoryAllocateInfo));

	pBuffer.buffer->bindMemory(*pBuffer.memory, 0);
}

void Vulkan::CreateImage(uint32_t pWidth, uint32_t pHeight, vk::Format pFormat, vk::ImageTiling pTiling, vk::ImageUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Image& pImage)
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

	pImage.image = Unique<vk::raii::Image>(_device->createImage(imageCreateInfo));

	vk::MemoryRequirements memReqs = pImage.image->getMemoryRequirements();
	vk::MemoryAllocateInfo memoryAllocateInfo
	{
		.allocationSize = memReqs.size,
		.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, pProps)
	};

	pImage.memory = Unique<vk::raii::DeviceMemory>(_device->allocateMemory(memoryAllocateInfo));
	pImage.image->bindMemory(*pImage.memory, 0);
}

void Vulkan::Init(RendererCreateInfo pCreateInfo)
{
	_info = pCreateInfo;

	CreateDXC();
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface(_info.windowHandle);
	PickPhysicalDevice();
	CreateDevice();
	CreateSwapchain();
	CreateSwapchainImageViews();
	CreateDescriptorSetLayout();
	CreateGraphicsPipelines();
	CreateCommandPool();
	//CreateDepthResources();
	//CreateTextureImage();
	//CreateTextureImageView();
	CreateTextureSampler();
	//CreateVertexBuffer();
	//CreateIndexBuffer();
	//CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffers();
	CreateSyncObjects();
}

bool Vulkan::StartFrame()
{
	_device->waitForFences(**_frameFinishedFence[_frameInFlightIdx], vk::True, UINT64_MAX);
	//_device->resetFences(**_frameFinishedFence[_frameInFlightIdx]);

	try
	{
		auto result = _swapchain->acquireNextImage(UINT64_MAX, *_imageAcquiredSemaphores[_frameInFlightIdx]);

		_activeImageIdx = *result;

		if (result.result == vk::Result::eSuboptimalKHR)
		{
			RecreateSurfaceAndSwapchain();
			return false;
		}
	}
	catch (const vk::OutOfDateKHRError&)
	{
		RecreateSurfaceAndSwapchain();
		return false;
	}
	catch (const vk::SurfaceLostKHRError&)
	{
		RecreateSurfaceAndSwapchain();
		return false;
	}	//_activeImageIdx = imageIdx;


	vk::CommandBufferBeginInfo beginInfo
	{
		.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
	};

	vk::raii::CommandBuffer& commandBuffer = *_commandBuffers[_frameInFlightIdx];

	commandBuffer.reset();
	commandBuffer.begin(beginInfo);

	return true;
}

void Vulkan::BlitToSwapchain(RGImage& pImage)
{
	if (!pImage.image.image)
		throw std::runtime_error("EndFrame received a null render image");

	vk::raii::CommandBuffer& commandBuffer = *_commandBuffers[_frameInFlightIdx];

	TransitionImageLayout(*_commandBuffers[_frameInFlightIdx], vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, _swapchainImages[_activeImageIdx]);
	TransitionImageLayout(*_commandBuffers[_frameInFlightIdx], pImage.currentLayout, vk::ImageLayout::eTransferSrcOptimal, *pImage.image.image, pImage.aspect);

	pImage.currentLayout = vk::ImageLayout::eTransferSrcOptimal;
	//blit final color for now
	vk::ImageSubresourceLayers subresource
	{
		.aspectMask = vk::ImageAspectFlagBits::eColor,
		.mipLevel = 0,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	vk::ImageBlit blit
	{
		.srcSubresource = subresource,
		.dstSubresource = subresource,
	};

	vk::Offset3D blitOffset0{ 0, 0, 0 };
	//vk::Offset3D blitOffsetSrc1{ (int32_t)renderWidth, (int32_t)renderHeight, 1 };
	vk::Offset3D blitOffsetDst1{ (int32_t)_swapchainExtent.width, (int32_t)_swapchainExtent.height, 1 };
	blit.srcOffsets[0] = blitOffset0;
	blit.srcOffsets[1] = blitOffsetDst1;
	blit.dstOffsets[0] = blitOffset0;
	blit.dstOffsets[1] = blitOffsetDst1;

	//auto finalColor = _graph.GetResource("FinalColor");
	_commandBuffers[_frameInFlightIdx]->blitImage(*pImage.image.image, vk::ImageLayout::eTransferSrcOptimal, _swapchainImages[_activeImageIdx], vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

	TransitionImageLayout(*_commandBuffers[_frameInFlightIdx], vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, *pImage.image.image, pImage.aspect);

	pImage.currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	TransitionImageLayout(*_commandBuffers[_frameInFlightIdx], vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eColorAttachmentOptimal, _swapchainImages[_activeImageIdx]);
}

void Vulkan::EndFrame()
{
	vk::raii::CommandBuffer& commandBuffer = *_commandBuffers[_frameInFlightIdx];
	TransitionImageLayout(*commandBuffer, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, _swapchainImages[_activeImageIdx]);
	commandBuffer.end();

	_device->resetFences(**_frameFinishedFence[_frameInFlightIdx]);

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eTransfer);

	const vk::SubmitInfo submitInfo
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &**_imageAcquiredSemaphores[_frameInFlightIdx],
		.pWaitDstStageMask = &waitDestinationStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &**_commandBuffers[_frameInFlightIdx],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &**_presentationReadySemaphore[_activeImageIdx]
	};

	_queue->submit(submitInfo, *_frameFinishedFence[_frameInFlightIdx]);

	const vk::PresentInfoKHR presentInfoKHR
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &**_presentationReadySemaphore[_activeImageIdx],
		.swapchainCount = 1,
		.pSwapchains = &**_swapchain,
		.pImageIndices = &_activeImageIdx
	};

	try
	{
		vk::Result result = _queue->presentKHR(presentInfoKHR);

		if (result == vk::Result::eSuboptimalKHR)
		{
			RecreateSwapchain();
			return;
		}
	}
	catch (const vk::OutOfDateKHRError&)
	{
		RecreateSurfaceAndSwapchain();
		return;
	}
	catch (const vk::SurfaceLostKHRError&)
	{
		RecreateSurfaceAndSwapchain();
		return;
	}

	_frameInFlightIdx = (_frameInFlightIdx + 1) % MaxFramesInFlight;
}

Buffer Vulkan::CreateVertexBuffer(void* pData, uint64_t pSize)
{
	Buffer vertexBuffer;

	Buffer staging;
	CreateBuffer(pSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);

	void* stagingData = staging.memory->mapMemory(0, pSize);
	memcpy(stagingData, pData, pSize);
	staging.memory->unmapMemory();

	CreateBuffer(pSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer);

	CopyBuffer(*staging.buffer, *vertexBuffer.buffer, pSize);

	return vertexBuffer;
}

Buffer Vulkan::CreateIndexBuffer(void* pData, uint64_t pSize)
{
	Buffer indexBuffer;

	Buffer staging;
	CreateBuffer(pSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);

	void* stagingData = staging.memory->mapMemory(0, pSize);
	memcpy(stagingData, pData, pSize);
	staging.memory->unmapMemory();

	CreateBuffer(pSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer);

	CopyBuffer(*staging.buffer, *indexBuffer.buffer, pSize);

	return indexBuffer;
}

Buffer Vulkan::CreateUniformBuffer(void* pData, uint64_t pSize)
{
	Buffer uniformBuffer;

	CreateBuffer(pSize, vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uniformBuffer);

	if (!pData) return uniformBuffer;

	Buffer stagingBuffer;

	CreateBuffer(pSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer);

	void* mappedData = stagingBuffer.memory->mapMemory(0, pSize);
	std::memcpy(mappedData, pData, pSize);
	stagingBuffer.memory->unmapMemory();

	CopyBuffer(*stagingBuffer.buffer, *uniformBuffer.buffer, pSize);

	return uniformBuffer;
}

Buffer Vulkan::CreateStorageBuffer(void* pData, uint64_t pSize)
{
	Buffer storageBuffer;

	CreateBuffer(pSize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, storageBuffer);

	if (!pData) return storageBuffer;

	Buffer stagingBuffer;

	CreateBuffer(pSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer);

	void* mappedData = stagingBuffer.memory->mapMemory(0, pSize);
	std::memcpy(mappedData, pData, pSize);
	stagingBuffer.memory->unmapMemory();

	CopyBuffer(*stagingBuffer.buffer, *storageBuffer.buffer, pSize);

	return storageBuffer;
}

RGBuffer Vulkan::CreateRenderBuffer(void* pData, uint64_t pSize, vk::BufferUsageFlags pUsage)
{
	RGBuffer buffer;

	if (pUsage & vk::BufferUsageFlagBits::eUniformBuffer)
	{
		buffer.buffer = CreateUniformBuffer(pData, pSize);
	}
	else if (pUsage & vk::BufferUsageFlagBits::eStorageBuffer)
	{
		buffer.buffer = CreateStorageBuffer(pData, pSize);
	}

	return buffer;
}

void Vulkan::MapBufferData(void* pData, uint64_t pSize, Buffer* pBuffer)
{
	if (!pData) IMGN_WARN("Data trying to be mapped is null");

	std::memcpy(pBuffer->memory->mapMemory(0, pSize), pData, pSize);
	pBuffer->memory->unmapMemory();
}

vk::raii::Semaphore Vulkan::CreateVkSemaphore()
{
	return _device->createSemaphore(vk::SemaphoreCreateInfo());
}

Image Vulkan::CreateDepthImage(uint32_t pWidth, uint32_t pHeight)
{
	Image depth;

	CreateImage(pWidth, pHeight, vk::Format::eD32Sfloat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depth);
	CreateImageView(vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth, depth);

	return depth;
}

Image Vulkan::CreateTextureImage(uint32_t pWidth, uint32_t pHeight, const uint8_t* pData)
{
	vk::DeviceSize imageSize = pWidth * pHeight * 4;

	Buffer staging;
	CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);

	void* data = staging.memory->mapMemory(0, imageSize);
	memcpy(data, pData, imageSize);
	staging.memory->unmapMemory();

	Image texture;
	CreateImage(pWidth, pHeight, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, texture);
	CreateImageView(vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, texture);

	TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, *texture.image);
	CopyBufferToImage(pWidth, pHeight, *staging.buffer, *texture.image);
	TransitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, *texture.image);

	return texture;
}

Image Vulkan::CreateTextureImage(const std::string& pFile)
{
	int width, height, channels;

	stbi_set_flip_vertically_on_load(true);
	uint8_t* pixels = stbi_load(pFile.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	vk::DeviceSize imageSize = width * height * 4;

	Buffer staging;
	CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);

	void* data = staging.memory->mapMemory(0, imageSize);
	memcpy(data, pixels, imageSize);
	staging.memory->unmapMemory();

	stbi_image_free(pixels);

	Image texture;
	CreateImage(static_cast<int>(width), static_cast<int>(height), vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, texture);
	CreateImageView(vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, texture);

	TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, *texture.image);
	CopyBufferToImage(static_cast<int>(width), static_cast<int>(height), *staging.buffer, *texture.image);
	TransitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, *texture.image);

	return texture;
}

RGImage Vulkan::CreateRenderImage(uint32_t pWidth, uint32_t pHeight, vk::Format pFormat, vk::ImageAspectFlags pAspect)
{
	RGImage image;
	image.aspect = pAspect;

	if (pAspect & vk::ImageAspectFlagBits::eDepth)
	{
		CreateImage(pWidth, pHeight, pFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, image.image);
		CreateImageView(pFormat, pAspect, image.image);

		return image;
	}

	CreateImage(pWidth, pHeight, pFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eDeviceLocal, image.image);
	CreateImageView(pFormat, pAspect, image.image);

	return image;
}

void Vulkan::UpdateImageDescriptor(uint32_t pSlot, const Image& pImage)
{
	if (pSlot >= NumDescriptorsStreaming)
	{
		IMGN_FATAL("Image slot {} is greater than NumDescriptorStreaming {}", pSlot, NumDescriptorsStreaming);
		throw std::runtime_error("Image descriptor slot exceeds NumDescriptorsStreaming");
	}
	if (!pImage.view)
	{
		IMGN_FATAL("Image view is null");
		throw std::runtime_error("Cannot update descriptor with a null image view");
	}

	vk::DescriptorImageInfo imageInfo
	{
		.sampler = **_textureSampler,
		.imageView = **pImage.view,
		.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
	};

	vk::WriteDescriptorSet write
	{
		.dstSet = **_textureDescriptorSet,
		.dstBinding = 0,
		.dstArrayElement = pSlot,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eCombinedImageSampler,
		.pImageInfo = &imageInfo
	};

	std::array writes = { write };

	_device->updateDescriptorSets(writes, {});
}

void Vulkan::TransitionImageLayout(vk::CommandBuffer pCommandBuffer, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::Image pImage, vk::ImageAspectFlags pAspect)
{
	vk::ImageMemoryBarrier barrier
	{
		.oldLayout = pOldLayout,
		.newLayout = pNewLayout,
		.image = pImage,
		.subresourceRange = { pAspect, 0, 1, 0, 1 }
	};

	vk::PipelineStageFlags srcStage;
	vk::PipelineStageFlags dstStage;

	if (pOldLayout == vk::ImageLayout::eUndefined && pNewLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (pOldLayout == vk::ImageLayout::eTransferDstOptimal && pNewLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		srcStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	// 3. Color Attachment -> Transfer Source (Preparing your Lighting-Output for the Blit)
	else if (pOldLayout == vk::ImageLayout::eColorAttachmentOptimal && pNewLayout == vk::ImageLayout::eTransferSrcOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
		srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	// 4. Undefined/Present -> Color Attachment (Preparing Swapchain for rendering)
	else if ((pOldLayout == vk::ImageLayout::eUndefined || pOldLayout == vk::ImageLayout::ePresentSrcKHR) && pNewLayout == vk::ImageLayout::eColorAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eNone;
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}
	// 5. Color Attachment -> Present (Sending Swapchain to monitor)
	else if (pOldLayout == vk::ImageLayout::eColorAttachmentOptimal && pNewLayout == vk::ImageLayout::ePresentSrcKHR)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eNone;
		srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dstStage = vk::PipelineStageFlagBits::eBottomOfPipe;
	}
	// 6. Transfer Dest -> Present (After Blit to Swapchain)
	else if (pOldLayout == vk::ImageLayout::eTransferDstOptimal && pNewLayout == vk::ImageLayout::ePresentSrcKHR)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eNone;
		srcStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eBottomOfPipe;
	}
	else if (pOldLayout == vk::ImageLayout::eUndefined && pNewLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
	}
	else if (pOldLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal && pNewLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		srcStage = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader;
	}
	else if (pOldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && pNewLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		srcStage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader;
		dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
	}
	else if (pOldLayout == vk::ImageLayout::eColorAttachmentOptimal && pNewLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dstStage = vk::PipelineStageFlagBits::eComputeShader;
	}
	else if (pOldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && pNewLayout == vk::ImageLayout::eColorAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

		srcStage = vk::PipelineStageFlagBits::eComputeShader;
		dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}
	else if (pOldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && pNewLayout == vk::ImageLayout::eTransferSrcOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		srcStage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (pOldLayout == vk::ImageLayout::eTransferSrcOptimal && pNewLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		srcStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader;
	}
	else if (pOldLayout == vk::ImageLayout::eTransferDstOptimal && pNewLayout == vk::ImageLayout::eColorAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

		srcStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	pCommandBuffer.pipelineBarrier(srcStage, dstStage, {}, nullptr, nullptr, barrier);
}

void Vulkan::TransitionImageLayout(vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::Image pImage, vk::ImageAspectFlags pAspect)
{
	unique<vk::raii::CommandBuffer> commandBuffer = StartSingleTimeCommand(); // Returns your unique/raii object

	// Pass the raw handle (*commandBuffer) to the worker
	TransitionImageLayout(*commandBuffer, pOldLayout, pNewLayout, pImage, pAspect);

	EndSingleTimeCommand(*commandBuffer);
}

void Vulkan::TransitionBuffer(vk::PipelineStageFlags2 pNewStage, vk::AccessFlags2 pNewAccess, Buffer& pBuffer)
{
}

unique<vk::raii::CommandBuffer> Vulkan::StartSingleTimeCommand()
{
	vk::CommandBufferAllocateInfo allocInfo
	{
		.commandPool = *_commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};

	unique<vk::raii::CommandBuffer> commandBuffer = Unique<vk::raii::CommandBuffer>(std::move(_device->allocateCommandBuffers(allocInfo).front()));

	vk::CommandBufferBeginInfo beginInfo
	{
		.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
	};

	commandBuffer->reset();
	commandBuffer->begin(beginInfo);

	return commandBuffer;
}

void Vulkan::EndSingleTimeCommand(vk::raii::CommandBuffer& pCommandBuffer)
{
	pCommandBuffer.end();

	vk::SubmitInfo submitInfo
	{
		.commandBufferCount = 1,
		.pCommandBuffers = &*pCommandBuffer
	};

	_queue->submit(submitInfo, nullptr);
	_queue->waitIdle();
}

ImGui_ImplVulkan_InitInfo Vulkan::GetImGuiInitInfo()
{
	_imguiColorFormat = static_cast<VkFormat>(_swapchainSurfaceFormat.format);

	return ImGui_ImplVulkan_InitInfo
	{
		.ApiVersion = vk::ApiVersion14,
		.Instance = **_instance,
		.PhysicalDevice = **_physicalDevice,
		.Device = **_device,
		.QueueFamily = _queueIdx,
		.Queue = **_queue,
		//.DescriptorPool = *_descriptorPool,
		.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE,
		.MinImageCount = static_cast<uint32_t>(_swapchainImages.size()),
		.ImageCount = static_cast<uint32_t>(_swapchainImages.size()),
		//.PipelineCache = *_pipelines.pipelinecac,
		.PipelineInfoMain
		{
			.PipelineRenderingCreateInfo =
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &_imguiColorFormat,
			}
		},
		.UseDynamicRendering = true,
	};
}
