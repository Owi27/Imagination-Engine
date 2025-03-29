#include "pch.h"
#include "VulkanContext.h"

void VulkanContext::StartFrame()
{
	vkWaitForFences(_device, 1, &_fences[_currentFrame], true, UINT64_MAX);
	vkResetFences(_device, 1, &_fences[_currentFrame]);
	vkAcquireNextImageKHR(_device, _swapchain, 0, _presentCompleteSemaphores[_currentFrame], nullptr, &_currentFrame);
}

void VulkanContext::EndFrame()
{
	vkQueuePresentKHR(_graphicsQueue, &_presentInfo);
	_currentFrame = (_currentFrame + 1) % _maxFramesInFlight;
}

void VulkanContext::SubmitQueue(VkSemaphore& prevSemaphore, VkSemaphore& currSemaphore, VkCommandBuffer& commandBuffers, VkFence fence)
{
	_submitInfo.pWaitSemaphores = &prevSemaphore;
	_submitInfo.pSignalSemaphores = &currSemaphore;
	_submitInfo.commandBufferCount = 1;
	_submitInfo.pCommandBuffers = &commandBuffers;

	vkQueueSubmit(_graphicsQueue, 1, &_submitInfo, fence);
}

void VulkanContext::PresentInfo(VkSemaphore& semaphore)
{
	_presentInfo.swapchainCount = 1;
	_presentInfo.pSwapchains = &_swapchain;
	_presentInfo.pImageIndices = &_currentFrame; //change to current frame
	_presentInfo.pWaitSemaphores = &semaphore;
	_presentInfo.waitSemaphoreCount = 1;
}

VkFramebuffer VulkanContext::GetFrameBuffer(int idx)
{
	_vulkanSurface.GetSwapchainFramebuffer(idx, (void**)&_frameBuffer);
	return _frameBuffer;
}

VkWriteDescriptorSet VulkanContext::WriteDescriptorSet(VkDescriptorSet& destinationSet, std::vector<VkDescriptorSetLayoutBinding>& layoutBindings, unsigned int destinationBinding, unsigned int arrayElement) const
{
	VkWriteDescriptorSet writeDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;

	for (size_t i = 0; i < layoutBindings.size(); i++)
	{
		if (layoutBindings[i].binding == destinationBinding)
		{
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType = layoutBindings[i].descriptorType;
			writeDescriptorSet.dstBinding = destinationBinding;
			writeDescriptorSet.dstSet = destinationSet;
			writeDescriptorSet.dstArrayElement = arrayElement;
			return writeDescriptorSet;
		}
	}

	return writeDescriptorSet;
}

VkWriteDescriptorSet VulkanContext::WriteDescriptorSet(VkDescriptorSet& destinationSet, std::vector<VkDescriptorSetLayoutBinding>& layoutBindings, unsigned int destinationBinding, const VkDescriptorBufferInfo* descriptorBufferInfo, unsigned int arrayElement)
{
	VkWriteDescriptorSet writeDescriptorSet = WriteDescriptorSet(destinationSet, layoutBindings, destinationBinding, arrayElement);
	writeDescriptorSet.pBufferInfo = descriptorBufferInfo;
	return writeDescriptorSet;
}

VkWriteDescriptorSet VulkanContext::WriteDescriptorSet(VkDescriptorSet& destinationSet, std::vector<VkDescriptorSetLayoutBinding>& layoutBindings, unsigned int destinationBinding, const VkDescriptorImageInfo* descriptorImageInfo, unsigned int arrayElement)
{
	VkWriteDescriptorSet writeDescriptorSet = WriteDescriptorSet(destinationSet, layoutBindings, destinationBinding, arrayElement);
	writeDescriptorSet.pImageInfo = descriptorImageInfo;
	return writeDescriptorSet;
}

VkPipeline VulkanContext::CreateGraphicsPipeline(PipelineDescription pipelineDescription, VkPipelineLayout& pipelineLayout, unsigned colorAttachmentCount)
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
			.stage = pipelineDescription.fragmentShader->GetShaderStageFlagBits(),
			.module = pipelineDescription.fragmentShader->GetShaderModule(),
			.pName = "main"
		},
		//vertex
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = pipelineDescription.vertexShader->GetShaderStageFlagBits(),
			.module = pipelineDescription.vertexShader->GetShaderModule(),
			.pName = "main"
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
		std::array<VkVertexInputBindingDescription, 4> vertexInputBindingDescriptions;
		std::array<VkVertexInputAttributeDescription, 4> vertexInputAttributeDescriptions;
		VkVertexInputBindingDescription vertexInputBindingDescription;
		VkVertexInputAttributeDescription vertexInputAttributeDescription;

		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(vec3);
		vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexInputAttributeDescription.binding = 0;
		vertexInputAttributeDescription.location = 0;
		vertexInputAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescription.offset = 0;

		vertexInputBindingDescriptions[0] = std::move(vertexInputBindingDescription);
		vertexInputAttributeDescriptions[0] = std::move(vertexInputAttributeDescription);

		if (pipelineDescription.vertexInput & NORMAL)
		{
			vertexInputBindingDescription.binding = 1;
			vertexInputBindingDescription.stride = sizeof(vec3);
			vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			vertexInputAttributeDescription.binding = 1;
			vertexInputAttributeDescription.location = 1;
			vertexInputAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexInputAttributeDescription.offset = 0;

			vertexInputBindingDescriptions[1] = std::move(vertexInputBindingDescription);
			vertexInputAttributeDescriptions[1] = std::move(vertexInputAttributeDescription);

		}
		if (pipelineDescription.vertexInput & TEXCOORD)
		{
			vertexInputBindingDescription.binding = 2;
			vertexInputBindingDescription.stride = sizeof(vec2);
			vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			vertexInputAttributeDescription.binding = 2;
			vertexInputAttributeDescription.location = 2;
			vertexInputAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
			vertexInputAttributeDescription.offset = 0;

			vertexInputBindingDescriptions[2] = std::move(vertexInputBindingDescription);
			vertexInputAttributeDescriptions[2] = std::move(vertexInputAttributeDescription);
		}
		if (pipelineDescription.vertexInput & TANGENT)
		{
			vertexInputBindingDescription.binding = 3;
			vertexInputBindingDescription.stride = sizeof(vec4);
			vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			vertexInputAttributeDescription.binding = 3;
			vertexInputAttributeDescription.location = 3;
			vertexInputAttributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			vertexInputAttributeDescription.offset = 0;

			vertexInputBindingDescriptions[3] = std::move(vertexInputBindingDescription);
			vertexInputAttributeDescriptions[3] = std::move(vertexInputAttributeDescription);
		}

		pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = vertexInputBindingDescriptions.size();
		pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
		pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptions.size();
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
		.cullMode = pipelineDescription.cullMode,
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

	vkCreatePipelineLayout(_device, &pipelineDescription.pipelineLayoutCreateInfo, nullptr, &pipelineLayout);

	VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
		.colorAttachmentCount = colorAttachmentCount,
		.pColorAttachmentFormats = pipelineDescription.colorAttachmentFormats.data(),
		.depthAttachmentFormat = pipelineDescription.depthFormat,
		.stencilAttachmentFormat = pipelineDescription.depthFormat
	};

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

VkCommandBuffer& VulkanContext::Render(VkCommandBuffer& commandBuffer, std::vector<Texture*>& textures, Texture* depth, std::function<void(VkCommandBuffer&)> drawCalls)
{
	std::vector< VkRenderingAttachmentInfoKHR> colorRenderingAttachmentInfos;

	for (auto& texture : textures)
	{
		GvkHelper::transition_image_layout(_device, _commandPool, _graphicsQueue, 1, texture->GetImage(), texture->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkRenderingAttachmentInfoKHR renderingAttachmentInfo
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
			.imageView = texture->GetImageView(),
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = texture->GetClearColorValue(),
		};

		colorRenderingAttachmentInfos.push_back(std::move(renderingAttachmentInfo));
	}

	VkRenderingAttachmentInfoKHR depthRenderingAttachmentInfo = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR };
	if (depth)
	{
		GvkHelper::transition_image_layout(_device, _commandPool, _graphicsQueue, 1, depth->GetImage(), depth->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		depthRenderingAttachmentInfo.imageView = depth->GetImageView();
		depthRenderingAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		depthRenderingAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthRenderingAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthRenderingAttachmentInfo.clearValue = { .depthStencil = {1.f, 0} };
		depthRenderingAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
	}

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
		.pDepthAttachment = depth ? &depthRenderingAttachmentInfo : nullptr
	};

	vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);

	VkViewport viewport = { 0, 0, static_cast<float>(_width), static_cast<float>(_height), 0, 1 };
	VkRect2D scissor = { {0, 0}, {_width, _height} };

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	drawCalls(commandBuffer);
	vkCmdEndRenderingKHR(commandBuffer);

	for (auto& texture : textures)
	{
		//GvkHelper::transition_image_layout(_device, _commandPool, _graphicsQueue, 1, texture->GetImage(), texture->GetFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	vkEndCommandBuffer(commandBuffer);
	return commandBuffer;
}
