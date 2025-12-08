#include "D:/GitHub/Imagination-Engine/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
#include "PipelineBuilder.h"

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddShaders(std::vector<std::pair<std::string, ShaderType>> shaders)
{
	_shaders.clear();
	_pipelineShaderStageCreateInfos.clear();
	_shaders.reserve(shaders.size());
	_pipelineShaderStageCreateInfos.reserve(shaders.size());

	for (auto& shader : shaders)
	{
		// Construct Shader in-place (move-only class)
		_shaders.emplace_back(_vk->device, shader.first, shader.second);

		// Get stage info from the just-added shader
		VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo = Attempt(_shaders.back().GetPipelineShaderStageCreateInfo());
		_pipelineShaderStageCreateInfos.push_back(pipelineShaderStageCreateInfo);
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
	for (auto& pipelineAttachment : pipelineAttachments)
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

RETURN(VkPipeline) GraphicsPipelineBuilder::BuildPipeline(VkPipelineLayout& pipelineLayout)
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

	if (vkCreatePipelineLayout(_vk->device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout)) return std::unexpected("VulkanBackend.cpp | BuildPipeline() | vkCreatePipelineLayout");

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
		.layout = pipelineLayout,
		//.renderPass = ,
		//.subpass = ,
		//.basePipelineHandle = ,
		//.basePipelineIndex = ,
	};

	if (vkCreateGraphicsPipelines(_vk->device, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline)) return std::unexpected("VulkanBackend.cpp | BuildPipeline() | vkCreateGraphicsPipelines");

	return pipeline;
}