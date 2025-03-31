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

	//auto itr = find_if(_colorInputs.begin(), _colorInputs.end(), [&](const std::shared_ptr<Texture>& texx)
	//	{
	//		return texx->GetImage() == tex.GetImage();
	//	});

	//if (itr != _colorInputs.end())
	//{
	//	return *itr;
	//}

	auto itr = std::find_if(_colorInputs.begin(), _colorInputs.end(),
		[&tex](const Texture* other) {
			return *other == tex; // Use the overloaded == operator
		});

	if (itr != _colorInputs.end()) return tex;
	else _colorInputs.push_back(&tex);

	return tex;
}

void RenderPass::AddTInput(const std::string& name)
{
	_colorInputs.push_back(_graph._blackboard.Get<Texture*>(name));
}

void RenderPass::AddUB(const std::string& name, void* data, unsigned size)
{
	_ubDataName = name;
	_graph._blackboard.Set<void*>(name, data);
	_uniformBufferOutput = _graph._blackboard.Set(name + " buffer", new Buffer(size, data, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT));
	_uniformBufferOutput->SetName(name + " buffer");
}

void RenderPass::AddVBOutput(const std::string& name, void* data, unsigned size)
{
	_graph._blackboard.Set<void*>(name, data);
	_bufferOutputs.push_back(_graph._blackboard.Set(name + " vertex buffer", new Buffer(size, data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)));
	_bufferOutputs.back()->SetName(name + " vertex buffer");
}

void RenderPass::AddIBOutput(const std::string& name, void* data, unsigned size)
{
	_graph._blackboard.Set<void*>(name, data);
	_bufferOutputs.push_back(_graph._blackboard.Set(name + " index buffer", new Buffer(size, data, VK_BUFFER_USAGE_INDEX_BUFFER_BIT)));
	_bufferOutputs.back()->SetName(name + " index buffer");
}

void RenderPass::UpdateUB(const std::string& name)
{
	auto& data = _graph._blackboard.Get<void*>(name);
	auto& buffer = _graph._blackboard.Get<Buffer*>(name + " buffer");

	buffer->WriteToBuffer(data);
}

void RenderPass::AddTOutput(const std::string& name)
{
	if (name.find("swapchain") != std::string::npos)
	{
		_renderToSwapchain = true;
	}
	else
	{
		_pipelineDescription.colorAttachmentFormats.push_back(VK_FORMAT_R16G16B16A16_UNORM);
		_colorOutputs.push_back(_graph._blackboard.Set(name, new Texture(VK_IMAGE_ASPECT_COLOR_BIT, VK_FORMAT_R16G16B16A16_UNORM)));
	}
}

void RenderPass::AddDOutput(const std::string& name)
{
	VkFormat depthFormat;
	std::vector<VkFormat> formats =
	{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	GvkHelper::find_depth_format(_vk.GetPhysicalDevice(), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, formats.data(), &depthFormat);
	_pipelineDescription.depthFormat = depthFormat;
	_depthStencilOutput = _graph._blackboard.Set(name, new Texture(VK_IMAGE_ASPECT_DEPTH_BIT, depthFormat));
}

Texture& RenderPass::AddTextureOutput(const std::string& name, const VkFormat format, const std::string& input)
{
	_graph._blackboard.Set(name, new Texture(VK_IMAGE_ASPECT_COLOR_BIT, VK_FORMAT_R16G16B16A16_UNORM));
	return _graph._blackboard.Get<Texture>(name);
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

	GvkHelper::find_depth_format(_vk.GetPhysicalDevice(), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, formats.data(), &depthFormat);
	tex.CreateImage({ _vk.GetWidth(), _vk.GetHeight(), 1 }, VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	tex.CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);

	_pipelineDescription.depthFormat = depthFormat;

	_depthStencilOutput = std::move(&tex);
	return *_depthStencilOutput;
}

Buffer& RenderPass::AddBufferInput(std::string name)
{
	auto& buffer = _graph.GetBufferResource(name);
	buffer.ReadInPass(_name);

	auto itr = std::find_if(_bufferInputs.begin(), _bufferInputs.end(),
		[&buffer](const Buffer* other) {
			return *other == buffer; // Use the overloaded == operator
		});
	if (itr != _bufferInputs.end()) return buffer;
	else _bufferInputs.push_back(&buffer);

	return buffer;
}

Buffer& RenderPass::AddBufferOutput(const std::string& name, unsigned size, void* data, const VkBufferUsageFlags usageFlags, const std::string& input)
{
	auto& buffer = _graph.GetBufferResource(name);
	buffer.WrittenInPass(_name);
	buffer.CreateBuffer(size, usageFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	buffer.WriteToBuffer(data);

	_bufferOutputs.push_back(&buffer);

	if (!input.empty())
	{
		auto& inputBuffer = _graph.GetBufferResource(input);
		inputBuffer.ReadInPass(_name);
		_bufferInputs.push_back(&inputBuffer);
	}
	else _bufferInputs.push_back(nullptr);

	return buffer;
}

Buffer& RenderPass::AddUniformBufferOutput(const std::string& name, unsigned size, void* data, const VkBufferUsageFlags usageFlags, const std::string& input)
{
	AddBufferOutput(name, size, data, usageFlags, input);
	_uniformBufferOutput = std::move(_bufferOutputs.back());
	_bufferOutputs.pop_back();

	return *_uniformBufferOutput;
}

Buffer& RenderPass::GetBuffer(const std::string& name)
{
	return _graph.GetBufferResource(name);
}

void RenderPass::Setup()
{
	if (_usingPushConstant)
	{
		_pipelineDescription.pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		_pipelineDescription.pipelineLayoutCreateInfo.pPushConstantRanges = &_pushConstantRange;
	}

	int i = 0;
	if (_descriptorPoolSizes.size() && _descriptorSetLayoutBindings.size())
	{
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = 1,
			.poolSizeCount = (unsigned)_descriptorPoolSizes.size(),
			.pPoolSizes = _descriptorPoolSizes.data(),
		};

		vkCreateDescriptorPool(_vk.GetDevice(), &descriptorPoolCreateInfo, nullptr, &_descriptorPool);

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = (unsigned)_descriptorSetLayoutBindings.size(),
			.pBindings = _descriptorSetLayoutBindings.data()
		};

		vkCreateDescriptorSetLayout(_vk.GetDevice(), &descriptorSetLayoutCreateInfo, nullptr, &_descriptorSetLayout);

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = _descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &_descriptorSetLayout
		};

		vkAllocateDescriptorSets(_vk.GetDevice(), &descriptorSetAllocateInfo, &_descriptorSet);

		_pipelineDescription.pipelineLayoutCreateInfo.setLayoutCount = 1;
		_pipelineDescription.pipelineLayoutCreateInfo.pSetLayouts = &_descriptorSetLayout;

		std::vector<VkWriteDescriptorSet> writeDescriptorSets;
		for (auto& layoutBinding : _descriptorSetLayoutBindings)
		{
			if (layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				VkDescriptorBufferInfo descriptorBufferInfo
				{
					.buffer = _uniformBufferOutput->GetBuffer(),
					.offset = 0,
					.range = _uniformBufferOutput->GetSize()
				};

				writeDescriptorSets.push_back(_vk.WriteDescriptorSet(_descriptorSet, _descriptorSetLayoutBindings, layoutBinding.binding, &descriptorBufferInfo));
			}

			if (layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				VkDescriptorImageInfo descriptorImageInfo
				{
					.sampler = _vk.GetSampler(),
					.imageView = _colorInputs[i++]->GetImageView(),
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				};

				writeDescriptorSets.push_back(_vk.WriteDescriptorSet(_descriptorSet, _descriptorSetLayoutBindings, layoutBinding.binding, &descriptorImageInfo));
			}
		}

		vkUpdateDescriptorSets(_vk.GetDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}

	_pipelineDescription.pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	_pipeline = _vk.CreateGraphicsPipeline(_pipelineDescription, _pipelineLayout, _colorOutputs.size());
}

void RenderPass::Execute()
{
	if (_uniformBufferOutput) UpdateUB(_ubDataName);

	for (auto& input : _colorInputs)
	{
		GvkHelper::transition_image_layout(_vk.GetDevice(), _vk.GetCommandPool(), _vk.GetGraphicsQueue(), 1, input->GetImage(), input->GetFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	for (auto& output : _colorOutputs)
	{
		GvkHelper::transition_image_layout(_vk.GetDevice(), _vk.GetCommandPool(), _vk.GetGraphicsQueue(), 1, _colorOutputs.back()->GetImage(), _colorOutputs.back()->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}

	vkResetCommandBuffer(_commandBuffer, 0);

	VkCommandBufferBeginInfo commandBufferBeginInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
	};

	vkBeginCommandBuffer(_commandBuffer, &commandBufferBeginInfo);

	vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
	if (_descriptorSet) vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet, 0, nullptr);

	if (_bufferOutputs.size() > 0)
	{
		std::vector<VkDeviceSize> offsets;
		std::vector<VkBuffer> vertexBuffers;
		VkBuffer indexBuffer;

		for (auto& output : _bufferOutputs)
		{
			if (output->GetName().find("vertex buffer") != std::string::npos)
			{
				vertexBuffers.push_back(output->GetBuffer());
				offsets.push_back(0);
			}
			else if (output->GetName().find("index buffer") != std::string::npos)
			{
				indexBuffer = output->GetBuffer();
			}
		}

		vkCmdBindVertexBuffers(_commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
		vkCmdBindIndexBuffer(_commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	}
	if (_renderToSwapchain) _vk.RenderToSwapchain(_commandBuffer, _colorOutputs, _depthStencilOutput, _drawCalls);
	else  _vk.Render(_commandBuffer, _colorOutputs, _depthStencilOutput, _drawCalls);
}

void RenderPass::SetPushConstantRange(VkPushConstantRange pushConstantRange)
{
	_usingPushConstant = true;
	_pushConstantRange = std::move(pushConstantRange);
}

void RenderPass::SetShaders(const std::string& shaderName)
{
	//pixel
	_shaders.push_back(std::make_shared<Shader>(shaderName + "FragmentShader", FRAGMENT_SHADER));
	_pipelineDescription.fragmentShader = _shaders.back().get();
	//vertex
	_shaders.push_back(std::make_shared<Shader>(shaderName + "VertexShader", VERTEX_SHADER));
	_pipelineDescription.vertexShader = _shaders.back().get();
}

void RenderPass::SetComputeShader(const std::string& shaderName)
{
	_shaders.push_back(std::make_shared<Shader>(shaderName + "ComputeShader", COMPUTE_SHADER));
}
