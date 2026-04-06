#include "D:/GitHub/Imagination-Engine/Refactor/Imagination - Current/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
#include "RenderPasses/GeometryPass.h"

void GeometryPass::BeginPass(vk::raii::CommandBuffer& pCommandBuffer)
{
	vk::RenderingAttachmentInfo colorAttachment
	{
		.imageView = gBuffer->GetColorImageView(),
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = vk::ClearColorValue(std::array<float, 4>{ 0.f, 0.f, 0.f, 1.f })
	};

	vk::RenderingAttachmentInfo depthAttachment
	{
		.imageView = gBuffer->GetDepthImageView(),
		.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = vk::ClearDepthStencilValue(1.0f, 0)
	};

	vk::RenderingInfoKHR renderingInfo
	{
		.renderArea = vk::Rect2D({0, 0}, {gBuffer->GetWidth(), gBuffer->GetHeight()}),
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachment,
		.pDepthAttachment = &depthAttachment
	};

	pCommandBuffer.beginRendering(renderingInfo);
}

void GeometryPass::Render(vk::raii::CommandBuffer& pCommandBuffer)
{
	//pCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
	//pCommandBuffer.setViewport(0, vk::Viewport(0.0f, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f));
	//pCommandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapchainExtent));


	//pCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, *_descriptorSets[DescriptorSetIndex(_frameIdx, RenderPassIdx::GBuffer)], nullptr);
	//pCommandBuffer.bindVertexBuffers(0, sponza->GetVertexBuffer(), { 0 });
	//pCommandBuffer.bindIndexBuffer(sponza->GetIndexBuffer(), 0, vk::IndexType::eUint16);

	//pCommandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

	//pCommandBuffer.endRendering();

}

void GeometryPass::EndPass(vk::raii::CommandBuffer& pCommandBuffer)
{
	pCommandBuffer.endRendering();
}
