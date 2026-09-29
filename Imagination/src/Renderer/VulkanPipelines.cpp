#include "pch.hpp"
#include "Vulkan.hpp"
#include "gltf/stb_image.h"
#include "EngineShaders.h"
#include "Imgn/UI/UIShader.h"

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

		std::array blendStates = { colorBlendAttachment, colorBlendAttachment, colorBlendAttachment, colorBlendAttachment, colorBlendAttachment };

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
			vk::Format::eR16G16Sfloat
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

	/* TAA */
	{
		vk::raii::ShaderModule computeSM = CreateShaderModule(CreateSPV(Shaders::TAAComputeShader, ComputeTarget));

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

		_pipelines.taaPipeline = Unique<vk::raii::Pipeline>(_device->createComputePipeline(nullptr, computePipelineCreateInfo));

	}

	//retained ui. alpha blended, no depth, one color target. does not share the gbuffer blend state.
	{
		const std::string pixelSource = std::string("#define UI_STAGE_PIXEL 1\n") + Shaders::UIShader;
		vk::raii::ShaderModule vertexSM = CreateShaderModule(CreateSPV(Shaders::UIShader, VertexTarget, L"VSMain"));
		vk::raii::ShaderModule fragmentSM = CreateShaderModule(CreateSPV(pixelSource, FragmentTarget, L"PSMain"));

		vk::PipelineShaderStageCreateInfo vertShaderStageInfo
		{
			.stage = vk::ShaderStageFlagBits::eVertex,
			.module = vertexSM,
			.pName = "VSMain"
		};

		vk::PipelineShaderStageCreateInfo fragShaderStageInfo
		{
			.stage = vk::ShaderStageFlagBits::eFragment,
			.module = fragmentSM,
			.pName = "PSMain"
		};

		std::array shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

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
			.depthTestEnable = vk::False,
			.depthWriteEnable = vk::False,
			.depthCompareOp = vk::CompareOp::eAlways,
			.depthBoundsTestEnable = vk::False,
			.stencilTestEnable = vk::False
		};

		vk::PipelineColorBlendAttachmentState uiBlend
		{
			.blendEnable = vk::True,
			.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
			.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
			.colorBlendOp = vk::BlendOp::eAdd,
			.srcAlphaBlendFactor = vk::BlendFactor::eOne,
			.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
			.alphaBlendOp = vk::BlendOp::eAdd,
			.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
		};

		vk::PipelineColorBlendStateCreateInfo colorBlending
		{
			.logicOpEnable = vk::False,
			.logicOp = vk::LogicOp::eCopy,
			.attachmentCount = 1,
			.pAttachments = &uiBlend
		};

		vk::Format uiFormat = vk::Format::eR16G16B16A16Sfloat;
		vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo
		{
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &uiFormat,
			.depthAttachmentFormat = vk::Format::eUndefined
		};

		vk::GraphicsPipelineCreateInfo uiPipelineInfo
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

		_pipelines.uiPipeline = Unique<vk::raii::Pipeline>(_device->createGraphicsPipeline(nullptr, uiPipelineInfo));
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
		vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute, nullptr),
		vk::DescriptorSetLayoutBinding(4, vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eCompute, nullptr)
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

