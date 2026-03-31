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
}

void GeometryPass::EndPass(vk::raii::CommandBuffer& pCommandBuffer)
{
	pCommandBuffer.endRendering();
}
