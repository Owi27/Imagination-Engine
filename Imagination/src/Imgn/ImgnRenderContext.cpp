#include "pch.hpp"
#include "ImgnRenderContext.h"

#include "ImgnRenderer.h"
#include "Imgn/ImgnRenderResources.h"


namespace Imgn
{
	vk::PipelineLayout RenderContext::GetPipelineLayout() const
	{
		return _renderer->GetPipelineLayout();
	}
	vk::RenderingAttachmentInfo RenderContext::CreateRenderingAttachmentInfo(std::string_view pName)
	{
		if (pName.contains("Depth"))
		{
			return vk::RenderingAttachmentInfo
			{
				.imageView = *_renderer->GetRenderGraphImage(pName).image.view,
				.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
				.loadOp = vk::AttachmentLoadOp::eClear,
				.storeOp = vk::AttachmentStoreOp::eStore,
				.clearValue = vk::ClearDepthStencilValue{ .0f, 0 }
			};
		}

		return vk::RenderingAttachmentInfo
		{
			.imageView = *_renderer->GetRenderGraphImage(pName).image.view,
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.clearValue = vk::ClearColorValue{ std::array<float, 4>{1.0f, 0.25f, 0.75f, 1.0f} }
		};
	}

	void RenderContext::BeginRendering(uint32_t pWidth, uint32_t pHeight, std::span<const vk::RenderingAttachmentInfo> pColors, const vk::RenderingAttachmentInfo* pDepth)
	{
		vk::RenderingInfo renderingInfo
		{
			.renderArea = { {0, 0}, { pWidth, pHeight } },
			.layerCount = 1,
			.colorAttachmentCount = static_cast<uint32_t>(pColors.size()),
			.pColorAttachments = pColors.data(),
			.pDepthAttachment = pDepth
		};

		_commandBuffer->beginRendering(renderingInfo);
	}
	void RenderContext::EndRendering()
	{
		_commandBuffer->endRendering();
	}
	void RenderContext::BindPipeline(vk::PipelineBindPoint pBindPoint, vk::Pipeline pPipeline)
	{
		_commandBuffer->bindPipeline(pBindPoint, pPipeline);
	}
	void RenderContext::BindDescriptorSet(vk::PipelineBindPoint pBindPoint, vk::PipelineLayout pPipelineLayout, int pFirstSet, vk::DescriptorSet pDescriptorSet)
	{
		_commandBuffer->bindDescriptorSets(pBindPoint, pPipelineLayout, pFirstSet, pDescriptorSet, {});
	}
	void RenderContext::SetViewport(uint32_t pWidth, uint32_t pHeight)
	{
		_commandBuffer->setViewport(0, vk::Viewport(0.0f, 0, static_cast<float>(pWidth), static_cast<float>(pHeight), 0.0f, 1.0f));
	}
	void RenderContext::SetScissor(uint32_t pWidth, uint32_t pHeight)
	{
		_commandBuffer->setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), { static_cast<float>(pWidth), static_cast<float>(pHeight) }));
	}
	vk::DescriptorBufferInfo RenderContext::CreateDescriptorBufferInfo(uint32_t pHandle, uint64_t pSize)
	{
		return vk::DescriptorBufferInfo
		{
			.buffer = *_renderer->GetBuffer(pHandle).buffer,
			.offset = 0,
			.range = pSize
		};
	}
	vk::DescriptorImageInfo RenderContext::CreateDescriptorImageInfo(std::string_view pName, vk::ImageLayout pLayout, vk::Sampler pSampler)
	{
		return vk::DescriptorImageInfo
		{
			.sampler = pSampler,
			.imageView = *_renderer->GetRenderGraphImage(pName).image.view,
			.imageLayout = pLayout,
		};
	}
	vk::WriteDescriptorSet RenderContext::CreateWriteDescriptorSet(uint32_t pBinding, vk::DescriptorType pDescriptorType, vk::DescriptorBufferInfo& pBufferInfo)
	{
		return vk::WriteDescriptorSet
		{
			.dstSet = nullptr,
			.dstBinding = pBinding,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = pDescriptorType,
			.pBufferInfo = &pBufferInfo
		};
	}
	vk::WriteDescriptorSet RenderContext::CreateWriteDescriptorSet(uint32_t pBinding, vk::DescriptorType pDescriptorType, vk::DescriptorImageInfo& pImageInfo)
	{
		return vk::WriteDescriptorSet
		{
			.dstSet = nullptr,
			.dstBinding = pBinding,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = pDescriptorType,
			.pImageInfo = &pImageInfo
		};
	}
	vk::WriteDescriptorSet RenderContext::CreateWriteDescriptorSet(uint32_t pBinding, vk::DescriptorType pDescriptorType, std::span<vk::DescriptorImageInfo> pImageInfo)
	{
		return vk::WriteDescriptorSet
		{
			.dstSet = nullptr,
			.dstBinding = pBinding,
			.dstArrayElement = 0,
			.descriptorCount = static_cast<uint32_t>(pImageInfo.size()),
			.descriptorType = pDescriptorType,
			.pImageInfo = pImageInfo.data()
		};
	}
	void RenderContext::PushDescriptorSet(vk::PipelineBindPoint pBindPoint, vk::PipelineLayout pPipelineLayout, std::span<vk::WriteDescriptorSet> pWrites)
	{
		_commandBuffer->pushDescriptorSet(pBindPoint, pPipelineLayout, 0, pWrites);
	}
	void RenderContext::Dispatch(uint32_t pX, uint32_t pY, uint32_t pZ)
	{
		_commandBuffer->dispatch(pX, pY, pZ);
	}
	void RenderContext::BindMesh(uint32_t pMeshHandle)
	{
		_commandBuffer->bindVertexBuffers(0, **_renderer->GetBuffer(_renderer->GetMesh(pMeshHandle).vertexBuffer).buffer, { 0 });
		_commandBuffer->bindIndexBuffer(**_renderer->GetBuffer(_renderer->GetMesh(pMeshHandle).indexBuffer).buffer, 0, vk::IndexType::eUint32);
	}
	void RenderContext::DrawPrimitive(const ImgnPrimitive& pPrimitive)
	{
		_commandBuffer->drawIndexed(pPrimitive.indexCount, 1, pPrimitive.firstIndex, pPrimitive.vertexOffset, 0);
	}
}