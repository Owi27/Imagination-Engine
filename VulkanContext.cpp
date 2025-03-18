#include "pch.h"
#include "VulkanContext.h"

VkFramebuffer VulkanContext::GetFrameBuffer(int idx)
{
	_vulkanSurface.GetSwapchainFramebuffer(idx, (void**)&_frameBuffer);
	return _frameBuffer;
}

VkPipeline VulkanContext::CreateGraphicsPipeline(PipelineDescription pipelineDescription, VkPipelineLayout pipelineLayout, unsigned colorAttachmentCount)
{
	if (!pipelineDescription.vertexShader || !pipelineDescription.fragmentShader)
	{
		std::cout << "Missing a fragment or vertex shader\n";
		return nullptr;
	}

	VkPipeline outPipeline;

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[2] =
	{
		//fragment
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = pipelineDescription.fragmentShader.get()->GetShaderStageFlagBits(),
			.module = pipelineDescription.fragmentShader.get()->GetShaderModule(),
			.pName = pipelineDescription.fragmentShader.get()->GetEntryPointName().c_str()
		},
		//vertex
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = pipelineDescription.vertexShader.get()->GetShaderStageFlagBits(),
			.module = pipelineDescription.vertexShader.get()->GetShaderModule(),
			.pName = pipelineDescription.vertexShader.get()->GetEntryPointName().c_str()
		}
	};

	//assembly state
	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.primitiveRestartEnable = false
	};

	switch (pipelineDescription.topology)
	{
	case POINT_TOPOLOGY:
		pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		break;
	case LINE_TOPOLOGY:
		pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		break;
	case LINE_STRIP_TOPOLOGY:
		pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		break;
	case TRIANGLE_TOPOLOGY:
		pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		break;
	case TRIANGLE_STRIP_TOPOLOGY:
		pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		break;
	default:
		break;
	}

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

	if (pipelineDescription.vertexInput & POSITION)
	{
		std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions(4);
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions(4);

		vertexInputBindingDescriptions[0].binding = 0;
		vertexInputBindingDescriptions[0].stride = sizeof(vec3);
		vertexInputBindingDescriptions[0].stride = sizeof(VK_VERTEX_INPUT_RATE_VERTEX);
		vertexInputAttributeDescriptions[0].binding = 0;
		vertexInputAttributeDescriptions[0].location = 0;
		vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescriptions[0].offset = 0;
		if (pipelineDescription.vertexInput & NORMAL)
		{
			vertexInputBindingDescriptions[1].binding = 1;
			vertexInputBindingDescriptions[1].stride = sizeof(vec3);
			vertexInputBindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			vertexInputAttributeDescriptions[1].binding = 1;
			vertexInputAttributeDescriptions[1].location = 1;
			vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexInputAttributeDescriptions[1].offset = 0;
		}
		if (pipelineDescription.vertexInput & TEXCOORD)
		{
			vertexInputBindingDescriptions[2].binding = 2;
			vertexInputBindingDescriptions[2].stride = sizeof(vec2);
			vertexInputBindingDescriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			vertexInputAttributeDescriptions[2].binding = 2;
			vertexInputAttributeDescriptions[2].location = 2;
			vertexInputAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			vertexInputAttributeDescriptions[2].offset = 0;
		}
		if (pipelineDescription.vertexInput & TANGENT)
		{
			vertexInputBindingDescriptions[3].binding = 3;
			vertexInputBindingDescriptions[3].stride = sizeof(vec4);
			vertexInputBindingDescriptions[3].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			vertexInputAttributeDescriptions[3].binding = 3;
			vertexInputAttributeDescriptions[3].location = 3;
			vertexInputAttributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			vertexInputAttributeDescriptions[3].offset = 0;

		}

		pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 4;
		pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
		pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 4;
		pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

	}
	else
	{
		pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
		pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
		pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
		pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;
	}

	VkViewport viewport
	{
		.x = 0,
		.y = 0,
		.width = static_cast<float>(_width),
		.height = static_cast<float>(_height),
		.minDepth = 0,
		.maxDepth = 1
	};

	VkRect2D scissor
	{
		.offset = {0, 0},
		.extent = {_width, _height}
	};

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor
	};

	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = false,
		.rasterizerDiscardEnable = false,
		.depthBiasEnable = false,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.f,
	};

	switch (pipelineDescription.polygonMode)
	{
	case FILL:
		pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		break;
	case LINE:
		pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_LINE;
		break;
	}

	switch (pipelineDescription.cullMode)
	{
	case FRONT:
		pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
		break;
	case BACK:
		pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		break;
	case NONE:
		pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
		break;
	default:
		break;
	}

	switch (pipelineDescription.frontFace)
	{
	case CLOCKWISE:
		pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		break;
	case COUNTER_CLOCKWISE:
		pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		break;
	}

	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = false,
		.minSampleShading = 1.0f,
		.pSampleMask = nullptr,
		.alphaToCoverageEnable = false,
		.alphaToOneEnable = false,
	};

	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
		.depthBoundsTestEnable = false,
		.stencilTestEnable = false,
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
	};

	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
	pipelineColorBlendAttachmentState.colorWriteMask = 0xF;
	pipelineColorBlendAttachmentState.blendEnable = false;
	pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
	pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
	pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
	pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	std::vector<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates;
	if (colorAttachmentCount > 1) pipelineColorBlendAttachmentStates.resize(colorAttachmentCount, pipelineColorBlendAttachmentState);
	else pipelineColorBlendAttachmentStates.push_back(std::move(pipelineColorBlendAttachmentState));

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = false,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = (unsigned)pipelineColorBlendAttachmentStates.size(),
		.pAttachments = pipelineColorBlendAttachmentStates.data(),
		.blendConstants = { {0.f}, {0.f},{0.f},{0.f}}
	};

	VkDynamicState dynamicState[2] =
	{
		// By setting these we do not need to re-create the pipeline on Resize
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamicState
	};

	VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
		.colorAttachmentCount = colorAttachmentCount,
		.pColorAttachmentFormats = pipelineDescription.colorAttachmentFormats.data(),
		.depthAttachmentFormat = pipelineDescription.depthFormat,
		.stencilAttachmentFormat = pipelineDescription.depthFormat
	};

	vkCreatePipelineLayout(_device, &pipelineDescription.pipelineLayoutCreateInfo, nullptr, &pipelineLayout);

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &pipelineRenderingCreateInfo,
		.stageCount = 2,
		.pStages = pipelineShaderStageCreateInfos,
		.pVertexInputState = &pipelineVertexInputStateCreateInfo,
		.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo,
		.pViewportState = &pipelineViewportStateCreateInfo,
		.pRasterizationState = &pipelineRasterizationStateCreateInfo,
		.pMultisampleState = &pipelineMultisampleStateCreateInfo,
		.pDepthStencilState = &pipelineDepthStencilStateCreateInfo,
		.pColorBlendState = &pipelineColorBlendStateCreateInfo,
		.pDynamicState = &pipelineDynamicStateCreateInfo,
		.layout = pipelineLayout
	};

	vkCreateGraphicsPipelines(_device, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &outPipeline);

	return outPipeline;
}

VkCommandBuffer& VulkanContext::Render(VkCommandBuffer& commandBuffer, std::vector<Texture*>& textures, Texture& depth, std::function<void(VkCommandBuffer&)> drawCalls)
{
	std::vector< VkRenderingAttachmentInfoKHR> colorRenderingAttachmentInfos;

	for (auto& texture : textures)
	{
		VkRenderingAttachmentInfoKHR renderingAttachmentInfo
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
			.imageView = texture->GetImageView(),
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = texture->GetClearColorValue()
		};

		colorRenderingAttachmentInfos.push_back(std::move(renderingAttachmentInfo));
	}

	VkRenderingAttachmentInfoKHR depthRenderingAttachmentInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
		.imageView = depth.GetImageView(),
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
	};

	VkRenderingInfo renderingInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea
		{
			.offset = {0, 0},
			.extent = {_width, _height}
		},
		.layerCount = 1,
		.colorAttachmentCount = (unsigned)colorRenderingAttachmentInfos.size(),
		.pColorAttachments = colorRenderingAttachmentInfos.data(),
		.pDepthAttachment = &depthRenderingAttachmentInfo
	};

	vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);

	VkViewport viewport = { 0, 0, static_cast<float>(_width), static_cast<float>(_height), 0, 1 };
	VkRect2D scissor = { {0, 0}, {_width, _height} };

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	drawCalls(commandBuffer);
	vkCmdEndRenderingKHR(commandBuffer);

	return commandBuffer;
}
