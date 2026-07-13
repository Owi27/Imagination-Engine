#include "pch.hpp"
#include "ImgnVulkan.hpp"
#include "gltf/stb_image.h"

static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
{
	if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
	{
		std::cout << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
	}

	return vk::False;
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

	_device = Unique<vk::raii::Device>(Device::Inst().GetPhysicalDevice(), deviceCreateInfo);
	_queue = Unique<vk::raii::Queue>(_device, _queueIdx, 0);
}

void ImgnVulkan::CreateSurface(void* pWindowHandle)
{
	VkSurfaceKHR surface;

	HWND hwnd = static_cast<HWND>(pWindowHandle);
	HINSTANCE* hInst = reinterpret_cast<HINSTANCE*>(GetWindowLongPtr(static_cast<HWND>(hwnd), GWLP_HINSTANCE));

	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo
	{
		.hinstance = *hInst ? *hInst : nullptr,
		.hwnd = hwnd
	};

	_surface = Unique<vk::raii::SurfaceKHR>(_instance, surfaceCreateInfo);
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

	_instance = Unique<vk::raii::Instance>(_ctx, instanceCreateInfo);
}

void ImgnVulkan::CreateSwapchain()
{
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = Device::Inst().GetPhysicalDevice().getSurfaceCapabilitiesKHR(*_surface);
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

	_swapchain = Unique<vk::raii::SwapchainKHR>(Device::Inst().GetDevice(), swapchainCreateInfo);
	_swapchainImages = _swapchain.getImages();
}

void ImgnVulkan::CreateSwapchainImageViews()
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

	_commandPool = Unique<vk::raii::CommandPool>(_device->createCommandPool(poolInfo));
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
	if (!candidates.empty() && candidates.rbegin()->first > 0) _physicalDevice = Unique<vk::raii::PhysicalDevice>(candidates.rbegin()->second);
	else throw std::runtime_error("failed to find a suitable GPU!");
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

	_debugMessenger = Unique<vk::raii::DebugUtilsMessengerEXT>(_instance->createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT));
}

void ImgnVulkan::CreateGraphicsPipelines()
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
			IMGN_FATAL(ss.str());
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
		.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
		.offset = 0,
		.size = 128
	};

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo
	{
		.setLayoutCount = 1,
		.pSetLayouts = &**_descriptorSetLayout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pcr
	};

	_pipelines.pipelineLayout = Unique<vk::raii::PipelineLayout>(_device, pipelineLayoutInfo);

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
			.cullMode = vk::CullModeFlagBits::eBack,
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

		std::array colorAttachmentFormats = { vk::Format::eR8G8B8A8Unorm, vk::Format::eR8G8B8A8Unorm, vk::Format::eR8G8B8A8Unorm, vk::Format::eR8G8B8A8Unorm,  };

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

		//delete
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
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pColorBlendState = &colorBlending;

			//_pipelines.lightingPipeline = vk::raii::Pipeline(Device::Inst().GetDevice(), nullptr, pipelineInfo);
		}
	}
}

void ImgnVulkan::CreateDescriptorSetLayout()
{
	std::array bindings =
	{
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, nullptr),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, nullptr),
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eCombinedImageSampler, NumDescriptorsStreaming, vk::ShaderStageFlagBits::eFragment, nullptr)
	};

	std::array flags =
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

	_descriptorSetLayout = Unique<vk::raii::DescriptorSetLayout>(_device->createDescriptorSetLayout(layoutInfo));
}

void ImgnVulkan::CreateImageView(vk::Format pFormat, vk::ImageAspectFlags pAspectFlags, Image& pImage)
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

void ImgnVulkan::CreateTextureImage(uint32_t pWidth, uint32_t pHeight, const uint8_t* pData)
{
	//vk::DeviceSize imageSize = pWidth * pHeight * 4;

	//Buffer staging = {};
	//CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);

	//void* data = staging.memory.mapMemory(0, imageSize);
	//memcpy(data, pData, imageSize);
	//staging.memory.unmapMemory();

	//delete[] pData;

	//CreateImage(width, height, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, _texture);

	//TransitionImageLayout(_texture.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	//CopyBufferToImage(staging.buffer, _texture.image, static_cast<uint32_t>(pWidth), static_cast<uint32_t>(pHeight));
	//TransitionImageLayout(_texture.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}

void ImgnVulkan::CreateTextureImage(const std::string& pFile)
{
	int width, height, channels;

	stbi_set_flip_vertically_on_load(true);
	uint8_t* pixels = stbi_load(pFile.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	vk::DeviceSize imageSize = width * height * 4;

	//Buffer staging = {};
	//CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);

	//void* data = staging.memory.mapMemory(0, imageSize);
	//memcpy(data, pixels, imageSize);
	//staging.memory.unmapMemory();

	//stbi_image_free(pixels);

	//CreateImage(width, height, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, _texture);

	//TransitionImageLayout(_texture.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	//CopyBufferToImage(staging.buffer, _texture.image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	//TransitionImageLayout(_texture.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}