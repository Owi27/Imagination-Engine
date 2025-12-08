#pragma once
#include <vulkan/vulkan.h>
#include "Structs.h"
#include "Enums.h"
#include "Resource.hpp"

struct AttachmentDesc
{
	Texture* texture = nullptr;
	LoadOp loadOp = LoadOp::CLEAR;
	StoreOp storeOp = StoreOp::STORE;
	VkClearValue clearValue = {};
};

class RenderPass
{
	unsigned _width = 0, _height = 0;
	std::vector<AttachmentDesc> _colorAttachments;
	AttachmentDesc _depthAttachment;
	bool _hasDepthAttachment = false;
	//std::array<Texture, 3> _swapchain;// = { Texture(_vk), Texture(_vk), Texture(_vk) };

protected:
	VulkanContext& _vk;
	VkPipeline _pipeline;
	VkPipelineLayout _pipelineLayout;
	bool _renderingToSwapchain = false;

	virtual void Record(VkCommandBuffer pCommandBuffer) = 0;

public:
	//RenderPass() = defualt;

	RenderPass(VulkanContext& pCtx) : _vk(pCtx)
	{
	}

	virtual ~RenderPass() = default;

	void AddColorAttachment(const AttachmentDesc& pDesc)
	{
		_colorAttachments.push_back(pDesc);
	}

	AttachmentDesc& GetColorAttachment(int pIndex)
	{
		return _colorAttachments[pIndex];
	}

	void SetDepthAttachment(const AttachmentDesc& pDesc)
	{
		_depthAttachment = pDesc;
		_hasDepthAttachment = true;
	}

	void Execute(VkCommandBuffer pCommandBuffer)
	{
		std::vector<VkRenderingAttachmentInfo> colorAttachmentInfos(_colorAttachments.size());

		int i = 0;
		for (AttachmentDesc& colorAttachment : _colorAttachments)
		{
			VkRenderingAttachmentInfo renderingAttachmentInfo
			{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				//.pNext = ,
				.imageView = colorAttachment.texture->imageView,
				.imageLayout = (VkImageLayout)colorAttachment.texture->imageLayout,
				//.resolveMode = ,
				//.resolveImageView = ,
				//.resolveImageLayout = ,
				.loadOp = (VkAttachmentLoadOp)colorAttachment.loadOp,
				.storeOp = (VkAttachmentStoreOp)colorAttachment.storeOp,
				.clearValue = colorAttachment.clearValue,
			};

			colorAttachmentInfos[i] = renderingAttachmentInfo;
			i++;
		}

		std::optional<VkRenderingAttachmentInfo> depthAttachmentInfo;

		if (_hasDepthAttachment)
		{
			VkRenderingAttachmentInfo renderingAttachmentInfo
			{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				//.pNext = ,
				.imageView = _vk.depthImageView,
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				//.resolveMode = ,
				//.resolveImageView = ,
				//.resolveImageLayout = ,
				.loadOp = (VkAttachmentLoadOp)_depthAttachment.loadOp,
				.storeOp = (VkAttachmentStoreOp)_depthAttachment.storeOp,
				.clearValue = _depthAttachment.clearValue,
			};

			depthAttachmentInfo = renderingAttachmentInfo;
		}
		
		if (_width == 0 || _height == 0)
		{
			_width = _vk.win.GetWidth();
			_height = _vk.win.GetHeight();
		}
		
		VkExtent2D extent
		{
			.width = _width,
			.height = _height,
		};

		VkRenderingInfo renderingInfo
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			//.pNext = ,
			//.flags = ,
			.renderArea
			{
				.offset
				{
					.x = 0,
					.y = 0,
				},
				.extent = extent
			},
			.layerCount = 1,
			//.viewMask = ,
			.colorAttachmentCount = static_cast<unsigned>(colorAttachmentInfos.size()),
			.pColorAttachments = colorAttachmentInfos.data(),
			.pDepthAttachment = depthAttachmentInfo ? &*depthAttachmentInfo : nullptr,
			//.pStencilAttachment = ,
		};

		vkCmdBeginRendering(pCommandBuffer, &renderingInfo);
		Record(pCommandBuffer);
		vkCmdEndRendering(pCommandBuffer);
	}

	void SetSize(unsigned pWidth, unsigned pHeight)
	{
		_width = pWidth;
		_height = pHeight;
	}
};