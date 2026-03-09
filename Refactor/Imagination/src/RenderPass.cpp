#include "pch.h"
#include "RenderPass.h"
#undef LoadImage

void GBufferPass::InitPass()
{
	std::vector<std::pair<std::string, ShaderType>> shaders
	{
		{ "FragmentShader", ShaderType::FRAGMENT },
		{ "VertexShader", ShaderType::VERTEX }
	};

	normal.CreateImage(VkCtx::Instance().swapchainExtent, 1, SampleCount::SAMPLE_1BIT, PipelineFormat::COLOR2, ImageTiling::OPTIMAL, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, MemoryFlags::GPU).CreateImageView(ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
	albedo.CreateImage(VkCtx::Instance().swapchainExtent, 1, SampleCount::SAMPLE_1BIT, PipelineFormat::COLOR, ImageTiling::OPTIMAL, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, MemoryFlags::GPU).CreateImageView(ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
	emissive.CreateImage(VkCtx::Instance().swapchainExtent, 1, SampleCount::SAMPLE_1BIT, PipelineFormat::COLOR, ImageTiling::OPTIMAL, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, MemoryFlags::GPU).CreateImageView(ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
	aoRM.CreateImage(VkCtx::Instance().swapchainExtent, 1, SampleCount::SAMPLE_1BIT, PipelineFormat::COLOR, ImageTiling::OPTIMAL, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, MemoryFlags::GPU).CreateImageView(ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);

	AddColorAttachment(AttachmentDesc{ .texture = &normal, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE, .clearValue {.color = {{ 0.f, 0.f, 0.f, 1.f }}} });
	AddColorAttachment(AttachmentDesc{ .texture = &albedo, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE, .clearValue {.color = {{ 0.f, 0.f, 0.f, 1.f }}} });
	AddColorAttachment(AttachmentDesc{ .texture = &emissive, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE, .clearValue {.color = {{ 0.f, 0.f, 0.f, 1.f }}} });
	AddColorAttachment(AttachmentDesc{ .texture = &aoRM, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE, .clearValue {.color = {{ 0.f, 0.f, 0.f, 1.f }}} });
	SetDepthAttachment(AttachmentDesc{ .texture = VkCtx::Instance().depth.get(), .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE, .clearValue {.depthStencil = { 1.f, 0 }} });

	//vertex data
	gBufferBuffers[0].CreateBuffer(sizeof(SceneData), BufferUsage::UNIFORM | BufferUsage::DEVICE_ADDRESS, MemoryFlags::CPU | MemoryFlags::CPU2GPU).WriteToBuffer(&sceneData);
	gBufferBuffers[1].CreateBuffer(sizeof(Vertex) * model.vertices.size(), BufferUsage::STORAGE | BufferUsage::DEVICE_ADDRESS, MemoryFlags::CPU | MemoryFlags::CPU2GPU).WriteToBuffer(model.vertices.data());
	gBufferBuffers[2].CreateBuffer(sizeof(Material) * model.materials.size(), BufferUsage::STORAGE | BufferUsage::DEVICE_ADDRESS, MemoryFlags::CPU | MemoryFlags::CPU2GPU).WriteToBuffer(model.materials.data());
	//gBufferBuffers[3].CreateBuffer(model.)

	textures.resize(model.textures.size());

	for (size_t i = 0; i < model.textures.size(); i++)
	{
		textures[i].LoadImage({ static_cast<uint32_t>(model.textures[i].width), static_cast<uint32_t>(model.textures[i].height), 1 }, model.textures[i].component, model.textures[i].image.data());
	}

	gBufferDescriptor
		.AddLayoutBinding(0, DescriptorType::UNIFORM_BUFFER, ShaderStage::VERTEX)
		.AddLayoutBinding(1, DescriptorType::STORAGE_BUFFER, ShaderStage::VERTEX)
		.AddLayoutBinding(2, DescriptorType::STORAGE_BUFFER, ShaderStage::FRAGMENT)
		.AddLayoutBinding(3, DescriptorType::IMAGE_SAMPLER, ShaderStage::FRAGMENT, textures.size())
		.CreateDescriptorSetLayout()
		.PrepareDescriptorBuffers();

	for (int i = 0; i < VkCtx::Instance().maxFrame; i++)
	{
		gBufferDescriptor
			.AddAddressInfo(0, i, gBufferBuffers[0].GetDeviceAddress(), sizeof(SceneData), DescriptorType::UNIFORM_BUFFER)
			.AddAddressInfo(1, i, gBufferBuffers[1].GetDeviceAddress(), sizeof(Vertex) * model.vertices.size(), DescriptorType::STORAGE_BUFFER)
			.AddAddressInfo(2, i, gBufferBuffers[2].GetDeviceAddress(), sizeof(Material) * model.materials.size(), DescriptorType::STORAGE_BUFFER);

		for (int j = 0; j < textures.size(); j++)
		{
			gBufferDescriptor.AddAddressInfo(3, i, j, textures[j].imageView, DescriptorType::IMAGE_SAMPLER);
		}
	}

	//gBufferDescriptor.AddAddressInfo(0, gBufferBuffers[0].GetDeviceAddress(), gBufferBuffers[0].size, DescriptorType::UNIFORM_BUFFER);
	//gBufferDescriptor.AddAddressInfo(1, gBufferBuffers[1].GetDeviceAddress(), gBufferBuffers[1].size, DescriptorType::STORAGE_BUFFER);
	//gBufferDescriptor.AddAddressInfo(2, gBufferBuffers[2].GetDeviceAddress(), gBufferBuffers[2].size, DescriptorType::STORAGE_BUFFER);
	//gBufferDescriptor.AddAddressInfo(0, gBufferBuffers[0].GetDeviceAddress(), gBufferBuffers[0].size, DescriptorType::UNIFORM_BUFFER);

	GraphicsPipelineBuilder pipelineBuilder;

	_pipeline = pipelineBuilder
		.AddShaders({ { "FragmentShader", ShaderType::FRAGMENT }, { "VertexShader", ShaderType::VERTEX } })
		.AddDescriptorSetLayout(gBufferDescriptor.layout)
		.AddPushConstantRange(VkPushConstantRange{ .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .offset = 0, .size = sizeof(mat4) })
		.SetRenderingFormats({ PipelineFormat::COLOR2 , PipelineFormat::COLOR, PipelineFormat::COLOR, PipelineFormat::COLOR }, VkCtx::Instance().depth->format)
		.SetBlendAttachments({ PipelineAttachment(), PipelineAttachment(), PipelineAttachment(), PipelineAttachment() })
		.BuildPipeline(_pipelineLayout);

	idxBuffer.CreateBuffer(sizeof(uint32_t) * model.indices.size(), BufferUsage::INDEX, MemoryFlags::CPU | MemoryFlags::CPU2GPU).WriteToBuffer(model.indices.data());
}

void GBufferPass::Record(VkCommandBuffer pCommandBuffer)
{
	vkCmdBindPipeline(pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
	vkCmdSetCullMode(pCommandBuffer, VK_CULL_MODE_BACK_BIT);
	vkCmdSetDepthTestEnable(pCommandBuffer, true);
	vkCmdSetDepthWriteEnable(pCommandBuffer, true);
	vkCmdSetDepthCompareOp(pCommandBuffer, VK_COMPARE_OP_LESS_OR_EQUAL);
	vkCmdSetFrontFace(pCommandBuffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	vkCmdSetPrimitiveTopology(pCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	vkCmdBindIndexBuffer(pCommandBuffer, idxBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

	VkDescriptorBufferBindingInfoEXT descriptorBindingInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
		//.pNext = ,
		.address = gBufferDescriptor.descriptorBuffer->GetDeviceAddress(),
		.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
	};

	VkCtx::Instance().vkCmdBindDescriptorBuffersEXT(pCommandBuffer, 1, &descriptorBindingInfo);

	uint64_t bufferOffset = gBufferDescriptor.layoutSize * VkCtx::Instance().currentFrame;;
	uint32_t idx = 0;

	VkCtx::Instance().vkCmdSetDescriptorBufferOffsetsEXT(pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &idx, &bufferOffset);

	//bufferOffset = 1 * gBufferDescriptor.size

	//vkCmdDraw(pCommandBuffer, (uint32_t)model.vertices.size(), 1, 0, 0);
	for (size_t i = 0; i < model.drawInfo.size(); i++)
	{
		vkCmdDrawIndexed(pCommandBuffer, model.drawInfo[i].idxCount, model.drawInfo[i].instCount, model.drawInfo[i].firstIdx, 0, model.drawInfo[i].firstInst);
	}
}
