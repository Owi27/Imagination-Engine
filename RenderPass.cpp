#include "pch.h"
#include "RenderPass.h"

Texture& RenderPass::AddTextureInput(std::string name)
{
	//Texture& gottenTex;
	//graph.get texture
	//add to graphics queue through texture
	//read in passes from renderresource (this pass name)
	//gottenTex.ReadInPass(_name); 

	auto& tex = _graph.GetTextureResource(name);
	tex.ReadInPass(_name);

	if (std::find(_colorInputs.begin(), _colorInputs.end(), tex) != _colorInputs.end())
	{
		return tex;
	}
	else _colorInputs.push_back(std::make_shared<Texture>(&tex));

	return tex;
}

Texture& RenderPass::AddTextureOutput(const std::string& name, const VkFormat format, const std::string& input)
{
	auto& tex = _graph.GetTextureResource(name);
	tex.WrittenInPass(_name);
	tex.CreateImage({ _vk->GetWidth(), _vk->GetHeight(), 1 }, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	tex.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

	_colorOutputs.push_back(std::make_shared<Texture>(&tex));

	if (!input.empty())
	{
		auto& inputTex = _graph.GetTextureResource(input);
		inputTex.ReadInPass(_name);
		_colorInputs.push_back(std::make_shared<Texture>(&inputTex));
	}
	else _colorInputs.push_back(nullptr);

	return tex;
}

Texture& RenderPass::AddDepthOutput(const std::string& name)
{
	auto& tex = _graph.GetTextureResource("depth");
	tex.WrittenInPass(_name);

	VkFormat depthFormat;

	std::vector<VkFormat> formats =
	{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	GvkHelper::find_depth_format(_vk->GetPhysicalDevice(), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, formats.data(), &depthFormat);
	tex.CreateImage({ _vk->GetWidth(), _vk->GetHeight(), 1 }, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	tex.CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);

	return tex;
}

Buffer& RenderPass::AddBufferInput(std::string name)
{
	auto& buffer = _graph.GetBufferResource(name);
	buffer.ReadInPass(_name);

	if (std::find(_bufferInputs.begin(), _bufferInputs.end(), buffer) != _bufferInputs.end())
	{
		return buffer;
	}
	else _bufferInputs.push_back(std::make_shared<Buffer>(&buffer));

	return buffer;
}

Buffer& RenderPass::AddBufferOutput(const std::string& name, unsigned size, void* data, const VkBufferUsageFlags usageFlags, const std::string& input)
{
	auto& buffer = _graph.GetBufferResource(name);
	buffer.WrittenInPass(_name);
	buffer.CreateBuffer(size, usageFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	buffer.WriteToBuffer(data);

	_bufferOutputs.push_back(std::make_shared<Buffer>(&buffer));

	if (!input.empty())
	{
		auto& inputBuffer = _graph.GetBufferResource(input);
		inputBuffer.ReadInPass(_name);
		_bufferInputs.push_back(std::make_shared<Buffer>(&inputBuffer));
	}
	else _bufferInputs.push_back(nullptr);

	return buffer;
}

void RenderPass::Setup()
{
	if (_usingPushConstant)
	{
		_pipelineDescription.pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		_pipelineDescription.pipelineLayoutCreateInfo.pPushConstantRanges = &_pushConstantRange;
	}

	if (_descriptorPoolSizes.size() && _descriptorSetLayoutBindings.size())
	{
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = 1,
			.poolSizeCount = (unsigned)_descriptorPoolSizes.size(),
			.pPoolSizes = _descriptorPoolSizes.data(),
		};

		vkCreateDescriptorPool(_vk->GetDevice(), &descriptorPoolCreateInfo, nullptr, &_descriptorPool);

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = (unsigned)_descriptorSetLayoutBindings.size(),
			.pBindings = _descriptorSetLayoutBindings.data()
		};

		vkCreateDescriptorSetLayout(_vk->GetDevice(), &descriptorSetLayoutCreateInfo, nullptr, &_descriptorSetLayout);

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = _descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &_descriptorSetLayout
		};

		vkAllocateDescriptorSets(_vk->GetDevice(), &descriptorSetAllocateInfo, &_descriptorSet);

		_pipelineDescription.pipelineLayoutCreateInfo.setLayoutCount = 1;
		_pipelineDescription.pipelineLayoutCreateInfo.pSetLayouts = &_descriptorSetLayout;
	}

	_vk->CreateGraphicsPipeline(_pipelineDescription, _pipelineLayout, _colorOutputs.size());
}

void RenderPass::Execute()
{
	/*VkCommandBuffer commandBuffer;

	VkRenderingAttachmentInfoKHR renderingAttachmentInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR
	};

	VkRenderingInfo renderingInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea
		{
			.offset = {0, 0},
			.extent = {_colorInputs[0].get()->GetExtent().width, _colorInputs[0].get()->GetExtent().height}
		},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &renderingAttachmentInfo
	};

	vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);*/

	_vk->Render(_commandBuffer, _colorOutputs, *_depthStencilOutput, _drawCalls);
}

void RenderPass::SetPushConstantRange(VkPushConstantRange pushConstantRange)
{
	_usingPushConstant = true;
	_pushConstantRange = std::move(pushConstantRange);
}