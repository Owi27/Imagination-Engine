#include "pch.h"
#include "RenderPass.h"

//Texture& RenderPass::AddTextureInput(std::string name)
//{
//	//Texture& gottenTex;
//	//graph.get texture
//	//add to graphics queue through texture
//	//read in passes from renderresource (this pass name)
//	//gottenTex.ReadInPass(_name); 
//
//	auto& tex = _graph.GetTextureResource(name);
//	tex.ReadInPass(_name);
//
//	//auto itr = find_if(_colorInputs.begin(), _colorInputs.end(), [&](const std::shared_ptr<Texture>& texx)
//	//	{
//	//		return texx->GetImage() == tex.GetImage();
//	//	});
//
//	//if (itr != _colorInputs.end())
//	//{
//	//	return *itr;
//	//}
//
//	auto itr = std::find_if(_colorInputs.begin(), _colorInputs.end(),
//		[&tex](const Texture* other) {
//			return *other == tex; // Use the overloaded == operator
//		});
//
//	if (itr != _colorInputs.end()) return tex;
//	else _colorInputs.push_back(tex);
//
//	return tex;
//}

void RenderPass::AddTInput(const std::string& name)
{
	_colorInputs.push_back(*_graph._blackboard.Get<Texture*>(name));
	//_colorInputs.back().get().TransitionLayout();
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
	_bufferOutputs.push_back(*_graph._blackboard.Set(name + " vertex buffer", new Buffer(size, data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)));
	_bufferOutputs.back().get().SetName(name + " vertex buffer");
}

void RenderPass::AddIBOutput(const std::string& name, void* data, unsigned size)
{
	_graph._blackboard.Set<void*>(name, data);
	_bufferOutputs.push_back(*_graph._blackboard.Set(name + " index buffer", new Buffer(size, data, VK_BUFFER_USAGE_INDEX_BUFFER_BIT)));
	_bufferOutputs.back().get().SetName(name + " index buffer");
}

void RenderPass::UpdateUB(const std::string& name)
{
	auto& data = _graph._blackboard.Get<void*>(name);
	auto& buffer = _graph._blackboard.Get<Buffer*>(name + " buffer");

	buffer->WriteToBuffer(data);
}

void RenderPass::Update()
{
	if (_uniformBufferOutput) UpdateUB(_ubDataName);
}

void RenderPass::AddCubeMap(const std::string& name, std::vector<std::string> imagePaths)
{
	_cubeMaps.push_back(*_graph._blackboard.Set(name, std::make_shared<CubeMap>(imagePaths)));
}

void RenderPass::AddTOutput(const std::string& name, VkFormat format)
{
	if (name.find("swapchain") != std::string::npos)
	{

		if (!_vk.SwapchainImagesInitialized()) _vk.CreateSwapchainTextures();

		_pipelineDescription.colorAttachmentFormats.push_back(_vk.GetSwapchainFormat());
		_renderToSwapchain = true;
	}
	else
	{
		_pipelineDescription.colorAttachmentFormats.push_back(format);
		_colorOutputs.push_back(*_graph._blackboard.Set(name, new Texture(VK_IMAGE_ASPECT_COLOR_BIT, format)));
		_colorOutputs.back().get().SetName(name);
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
	_depth = _graph._blackboard.Set(name, new Texture(VK_IMAGE_ASPECT_DEPTH_BIT, depthFormat));
	GvkHelper::transition_image_layout(_vk.GetDevice(), _vk.GetCommandPool(), _vk.GetGraphicsQueue(), 1, _depth->GetImage(), _depth->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

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
		std::vector<VkDescriptorBufferInfo> bufferInfos;
		std::vector<VkDescriptorImageInfo> imageInfos;
		imageInfos.reserve(_colorInputs.size());
		bufferInfos.reserve(_bufferInputs.size());
		writeDescriptorSets.reserve(_descriptorSetLayoutBindings.size());
		int i = 0;
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

				bufferInfos.emplace_back(descriptorBufferInfo);

				writeDescriptorSets.push_back(_vk.WriteDescriptorSet(_descriptorSet, _descriptorSetLayoutBindings, layoutBinding.binding, &bufferInfos.back()));
			}

			if (layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				if (_colorInputs.size())
				{
					VkDescriptorImageInfo descriptorImageInfo
					{
						.sampler = _vk.GetSampler(),
						.imageView = _colorInputs[i++].get().GetImageView(),
						.imageLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR
					};

					imageInfos.emplace_back(descriptorImageInfo);
					writeDescriptorSets.push_back(_vk.WriteDescriptorSet(_descriptorSet, _descriptorSetLayoutBindings, layoutBinding.binding, &imageInfos.back()));
				}
				else if (_cubeMaps.size())
				{
					VkDescriptorImageInfo descriptorImageInfo
					{
						.sampler = _vk.GetSampler(),
						.imageView = _cubeMaps[0].get().GetCubeMapImageView(),
						.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
					};

					imageInfos.emplace_back(descriptorImageInfo);
					writeDescriptorSets.push_back(_vk.WriteDescriptorSet(_descriptorSet, _descriptorSetLayoutBindings, layoutBinding.binding, &imageInfos.back()));
				}
			}

			if (layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
			{
				VkDescriptorImageInfo descriptorImageInfo
				{
					.sampler = _vk.GetSampler(),
					.imageView = _colorInputs[i++].get().GetImageView(),
					.imageLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR
				};

				imageInfos.emplace_back(descriptorImageInfo);
				//pushback overwrites all 3 images for some reason
				writeDescriptorSets.push_back(_vk.WriteDescriptorSet(_descriptorSet, _descriptorSetLayoutBindings, layoutBinding.binding, &imageInfos.back()));
			}
		}

		vkUpdateDescriptorSets(_vk.GetDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}

	_pipelineDescription.pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	if (_colorOutputs.size() > 0) _pipeline = _vk.CreateGraphicsPipeline(_pipelineDescription, _pipelineLayout, _colorOutputs.size());
	else _pipeline = _vk.CreateGraphicsPipeline(_pipelineDescription, _pipelineLayout);
}

void RenderPass::BuildCommandBuffer()
{
	if (_uniformBufferOutput) UpdateUB(_ubDataName);

	std::vector<VkDeviceSize> offsets;
	std::vector<VkBuffer> vertexBuffers;
	VkBuffer indexBuffer = nullptr;
	if (_bufferOutputs.size() > 0)
	{
		for (auto& output : _bufferOutputs)
		{
			if (output.get().GetName().find("vertex buffer") != std::string::npos)
			{
				vertexBuffers.push_back(output.get().GetBuffer());
				offsets.push_back(0);
			}
			else if (output.get().GetName().find("index buffer") != std::string::npos)
			{
				indexBuffer = output.get().GetBuffer();
			}
		}
	}

	if (_renderToSwapchain)
	{
		_vk.RenderToSwapchain(_depth, _drawCalls, [this, vertexBuffers, indexBuffer, offsets](VkCommandBuffer& commandBuffer)
			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
				if (_descriptorSet) vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet, 0, nullptr);
				if (vertexBuffers.size() > 0) vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
				if (indexBuffer) vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
			});
	}
	else if (_colorOutputs.size())
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
		};

		vkBeginCommandBuffer(_commandBuffer, &commandBufferBeginInfo);

		vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
		if (_descriptorSet) vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet, 0, nullptr);
		if (vertexBuffers.size() > 0) vkCmdBindVertexBuffers(_commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
		if (indexBuffer) vkCmdBindIndexBuffer(_commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		_vk.Render(_commandBuffer, _colorOutputs, _depth, _drawCalls);
	}
	else if (_cubeMaps.size())
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
		};

		vkBeginCommandBuffer(_commandBuffer, &commandBufferBeginInfo);

		vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
		if (_descriptorSet) vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet, 0, nullptr);
		if (vertexBuffers.size() > 0) vkCmdBindVertexBuffers(_commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
		if (indexBuffer) vkCmdBindIndexBuffer(_commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		_vk.Render(_commandBuffer, _colorOutputs, _depth, _drawCalls);
	}
}

VkCommandBuffer& RenderPass::GetCommandBuffer()
{
	if (_renderToSwapchain)
	{
		return _vk.GetCurrentCommandBuffer();
	}

	return _commandBuffer;
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
