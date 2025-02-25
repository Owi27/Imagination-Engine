#include "pch.h"
#include "VulkanContext.h"
using namespace VkContext;

VkFramebuffer VulkanContext::GetFrameBuffer(int idx)
{
	_vulkanSurface.GetSwapchainFramebuffer(idx, (void**)&_frameBuffer);
	return _frameBuffer;
}

VkPipeline VkContext::VulkanContext::CreateGraphicsPipeline(PipelineDescription pipelineDescription)
{
	if (!pipelineDescription.vertexShader || !pipelineDescription.fragmentShader)
	{
		std::cout << "Missing a fragment or vertex shader\n";
		return nullptr;
	}

	//VkPipeline

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[2] =
	{
		//fragment
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = pipelineDescription.fragmentShader.get()->GetVkShaderStageFlagBits(),
			.module = pipelineDescription.fragmentShader.get()->GetVkShaderModule(),
			.pName = pipelineDescription.fragmentShader.get()->GetEntryPointName()
		},
		//vertex
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = pipelineDescription.vertexShader.get()->GetVkShaderStageFlagBits(),
			.module = pipelineDescription.vertexShader.get()->GetVkShaderModule(),
			.pName = pipelineDescription.vertexShader.get()->GetEntryPointName()
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


}
