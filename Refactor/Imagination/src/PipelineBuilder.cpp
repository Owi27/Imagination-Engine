#include "pch.h"
#include "PipelineBuilder.h"

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddShaders(std::vector<std::pair<std::string, ShaderType>> pShaders)
{
    _shaders.clear();
    _shaderStages.clear();

    _shaders.resize(pShaders.size());
    _shaderStages.resize(pShaders.size());

    int i = 0;
    for (auto& [file, type] : pShaders)
    {
        _shaders[i] = std::move(Shader(file, type));
        _shaderStages[i] = _shaders[i].GetPipelineShaderStageCreateInfo();

        i++;
    }

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddDescriptorSetLayout(VkDescriptorSetLayout pLayout)
{
    _setLayouts.push_back(pLayout);

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddPushConstantRange(VkPushConstantRange pPushConstantRange)
{
    _pushConstants.push_back(pPushConstantRange);

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetRenderingFormats(const std::vector<PipelineFormat>& pFormats, PipelineFormat pDepthFormat)
{
    _colorFormats.resize(pFormats.size());

    for (size_t i = 0; i < pFormats.size(); i++)
    {
        _colorFormats[i] = (VkFormat)pFormats[i];
    }

    _depthFormat = (VkFormat)pDepthFormat;

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetBlendAttachments(const std::vector<PipelineAttachment>& pAttachments)
{
	_blendAttachments.resize(pAttachments.size());

	int i = 0;
	for (auto& pipelineAttachment : pAttachments)
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

	return *this;
}

VkPipeline GraphicsPipelineBuilder::BuildPipeline(VkPipelineLayout& pOutLayout)
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo
    {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            //.flags = ,
            .setLayoutCount = static_cast<uint32_t>(_setLayouts.size()),
            .pSetLayouts = _setLayouts.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(_pushConstants.size()),
            .pPushConstantRanges = _pushConstants.data()
    };

    vkCreatePipelineLayout(VkCtx::Instance().device, &pipelineLayoutCreateInfo, nullptr, &pOutLayout);

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr
    };

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,  // dynamic
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
    };

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = static_cast<uint32_t>(_blendAttachments.size()),
        .pAttachments = _blendAttachments.data()
    };

    std::vector<VkDynamicState> dynamicStates = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_CULL_MODE_EXT,
        VK_DYNAMIC_STATE_FRONT_FACE_EXT,
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT,
        VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT,
        VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT,
        VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    VkPipelineRenderingCreateInfo renderingCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = static_cast<uint32_t>(_colorFormats.size()),
        .pColorAttachmentFormats = _colorFormats.data(),
        .depthAttachmentFormat = _depthFormat
    };

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingCreateInfo,
        .flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
        .stageCount = static_cast<uint32_t>(_shaderStages.size()),
        .pStages = _shaderStages.data(),
        .pVertexInputState = &vertexInputStateCreateInfo,
        .pInputAssemblyState = &inputAssemblyStateCreateInfo,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizationStateCreateInfo,
        .pMultisampleState = &multisampleStateCreateInfo,
        .pDepthStencilState = &depthStencilCreateInfo,
        .pColorBlendState = &colorBlendStateCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = pOutLayout,
        .renderPass = nullptr, // dynamic rendering
        .subpass = 0
    };

    VkPipeline pipeline;
    vkCreateGraphicsPipelines(VkCtx::Instance().device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline);

    return pipeline;
}
