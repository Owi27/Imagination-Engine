#include "pch.hpp"
#include "Vulkan.hpp"
#include "gltf/stb_image.h"
#include "EngineShaders.h"
#include "Imgn/UI/UIShader.h"

void Vulkan::TransitionImageLayout(vk::CommandBuffer pCommandBuffer, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::Image pImage, vk::ImageAspectFlags pAspect)
{
	vk::ImageMemoryBarrier barrier
	{
		.oldLayout = pOldLayout,
		.newLayout = pNewLayout,
		.image = pImage,
		.subresourceRange = { pAspect, 0, 1, 0, 1 }
	};

	vk::PipelineStageFlags srcStage;
	vk::PipelineStageFlags dstStage;

	if (pOldLayout == vk::ImageLayout::eUndefined && pNewLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (pOldLayout == vk::ImageLayout::eTransferDstOptimal && pNewLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		srcStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader;
	}
	// 3. Color Attachment -> Transfer Source (Preparing your Lighting-Output for the Blit)
	else if (pOldLayout == vk::ImageLayout::eColorAttachmentOptimal && pNewLayout == vk::ImageLayout::eTransferSrcOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
		srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	// 4. Undefined/Present -> Color Attachment (Preparing Swapchain for rendering)
	else if ((pOldLayout == vk::ImageLayout::eUndefined || pOldLayout == vk::ImageLayout::ePresentSrcKHR) && pNewLayout == vk::ImageLayout::eColorAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eNone;
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}
	// 5. Color Attachment -> Present (Sending Swapchain to monitor)
	else if (pOldLayout == vk::ImageLayout::eColorAttachmentOptimal && pNewLayout == vk::ImageLayout::ePresentSrcKHR)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eNone;
		srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dstStage = vk::PipelineStageFlagBits::eBottomOfPipe;
	}
	// 6. Transfer Dest -> Present (After Blit to Swapchain)
	else if (pOldLayout == vk::ImageLayout::eTransferDstOptimal && pNewLayout == vk::ImageLayout::ePresentSrcKHR)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eNone;
		srcStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eBottomOfPipe;
	}
	else if (pOldLayout == vk::ImageLayout::eUndefined && pNewLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
	}
	else if (pOldLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal && pNewLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		srcStage = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader;
	}
	else if (pOldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && pNewLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		srcStage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader;
		dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
	}
	else if (pOldLayout == vk::ImageLayout::eColorAttachmentOptimal && pNewLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		//fragment so the editor can sample UIComposite (and other color targets) after the graph
		dstStage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader;
	}
	else if (pOldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && pNewLayout == vk::ImageLayout::eColorAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

		srcStage = vk::PipelineStageFlagBits::eComputeShader;
		dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}
	else if (pOldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && pNewLayout == vk::ImageLayout::eTransferSrcOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		srcStage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (pOldLayout == vk::ImageLayout::eTransferSrcOptimal && pNewLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		srcStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader;
	}
	else if (pOldLayout == vk::ImageLayout::eTransferDstOptimal && pNewLayout == vk::ImageLayout::eColorAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

		srcStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}
	else if (pOldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && pNewLayout == vk::ImageLayout::eGeneral)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;

		srcStage = vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eFragmentShader;
		dstStage = vk::PipelineStageFlagBits::eComputeShader;
	}
	else if (pOldLayout == vk::ImageLayout::eGeneral && pNewLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		srcStage = vk::PipelineStageFlagBits::eComputeShader;
		dstStage = vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (pOldLayout == vk::ImageLayout::eUndefined && pNewLayout ==
		vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = {};

			barrier.dstAccessMask =
				vk::AccessFlagBits::eShaderRead;

			srcStage =
				vk::PipelineStageFlagBits::eTopOfPipe;

			dstStage =
				vk::PipelineStageFlagBits::eComputeShader;
				}
	else if (
		pOldLayout ==
		vk::ImageLayout::eShaderReadOnlyOptimal &&
		pNewLayout ==
		vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask =
				vk::AccessFlagBits::eShaderRead;

			barrier.dstAccessMask =
				vk::AccessFlagBits::eTransferWrite;

			srcStage =
				vk::PipelineStageFlagBits::eComputeShader |
				vk::PipelineStageFlagBits::eFragmentShader;

			dstStage =
				vk::PipelineStageFlagBits::eTransfer;
				}
	else if (
		pOldLayout == vk::ImageLayout::eUndefined &&
		pNewLayout == vk::ImageLayout::eGeneral)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask =
				vk::AccessFlagBits::eShaderRead |
				vk::AccessFlagBits::eShaderWrite;

			srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
			dstStage = vk::PipelineStageFlagBits::eComputeShader;
			}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	pCommandBuffer.pipelineBarrier(srcStage, dstStage, {}, nullptr, nullptr, barrier);
}

void Vulkan::TransitionImageLayout(vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::Image pImage, vk::ImageAspectFlags pAspect)
{
	unique<vk::raii::CommandBuffer> commandBuffer = StartSingleTimeCommand(); // Returns your unique/raii object

	// Pass the raw handle (*commandBuffer) to the worker
	TransitionImageLayout(*commandBuffer, pOldLayout, pNewLayout, pImage, pAspect);

	EndSingleTimeCommand(*commandBuffer);
}

void Vulkan::TransitionBuffer(vk::PipelineStageFlags2 pNewStage, vk::AccessFlags2 pNewAccess, Buffer& pBuffer)
{
}

unique<vk::raii::CommandBuffer> Vulkan::StartSingleTimeCommand()
{
	vk::CommandBufferAllocateInfo allocInfo
	{
		.commandPool = *_commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};

	unique<vk::raii::CommandBuffer> commandBuffer = Unique<vk::raii::CommandBuffer>(std::move(_device->allocateCommandBuffers(allocInfo).front()));

	vk::CommandBufferBeginInfo beginInfo
	{
		.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
	};

	commandBuffer->reset();
	commandBuffer->begin(beginInfo);

	return commandBuffer;
}

void Vulkan::EndSingleTimeCommand(vk::raii::CommandBuffer& pCommandBuffer)
{
	pCommandBuffer.end();

	vk::SubmitInfo submitInfo
	{
		.commandBufferCount = 1,
		.pCommandBuffers = &*pCommandBuffer
	};

	_queue->submit(submitInfo, nullptr);
	_queue->waitIdle();
}

ImGui_ImplVulkan_InitInfo Vulkan::GetImGuiInitInfo()
{
	_imguiColorFormat = static_cast<VkFormat>(_swapchainSurfaceFormat.format);

	return ImGui_ImplVulkan_InitInfo
	{
		.ApiVersion = vk::ApiVersion14,
		.Instance = **_instance,
		.PhysicalDevice = **_physicalDevice,
		.Device = **_device,
		.QueueFamily = _queueIdx,
		.Queue = **_queue,
		//.DescriptorPool = *_descriptorPool,
		.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE,
		.MinImageCount = static_cast<uint32_t>(_swapchainImages.size()),
		.ImageCount = static_cast<uint32_t>(_swapchainImages.size()),
		//.PipelineCache = *_pipelines.pipelinecac,
		.PipelineInfoMain
		{
			.PipelineRenderingCreateInfo =
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &_imguiColorFormat,
			}
		},
		.UseDynamicRendering = true,
	};
}

void Vulkan::CopyRenderImage(RGImage& pSrc, RGImage& pDst)
{
	vk::CommandBuffer cmd = *_commandBuffers[_frameInFlightIdx];

	const vk::ImageLayout oldSrcLayout = pSrc.currentLayout;

	const vk::ImageLayout oldDstLayout = pDst.currentLayout;


	// Source -> transfer source
	TransitionImageLayout(cmd, oldSrcLayout, vk::ImageLayout::eTransferSrcOptimal, *pSrc.image.image, pSrc.aspect);

	pSrc.currentLayout = vk::ImageLayout::eTransferSrcOptimal;


	// History -> transfer destination
	TransitionImageLayout(
		cmd,
		oldDstLayout,
		vk::ImageLayout::eTransferDstOptimal,
		*pDst.image.image,
		pDst.aspect
	);

	pDst.currentLayout =
		vk::ImageLayout::eTransferDstOptimal;


	vk::ImageCopy region
	{
		.srcSubresource =
		{
			vk::ImageAspectFlagBits::eColor,
			0,
			0,
			1
		},

		.srcOffset =
		{
			0,
			0,
			0
		},

		.dstSubresource =
		{
			vk::ImageAspectFlagBits::eColor,
			0,
			0,
			1
		},

		.dstOffset =
		{
			0,
			0,
			0
		},

		.extent =
		{
			pSrc.width,
			pSrc.height,
			1
		}
	};

	cmd.copyImage(*pSrc.image.image, vk::ImageLayout::eTransferSrcOptimal, *pDst.image.image, vk::ImageLayout::eTransferDstOptimal,	region);


	// TAAResolved will potentially be displayed/read.
	TransitionImageLayout(cmd, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, *pSrc.image.image, pSrc.aspect);

	pSrc.currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;


	// History must be readable next frame.
	TransitionImageLayout(cmd, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, *pDst.image.image, pDst.aspect);

	pDst.currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
}

void Vulkan::ClearSwapchain()
{
	auto& cmd = *_commandBuffers[_frameInFlightIdx];
	const vk::Image image = _swapchainImages[_activeImageIdx];

	TransitionImageLayout(cmd, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, image);

	const vk::ClearColorValue color{ std::array<float, 4>{ 0.04f, 0.04f, 0.04f, 1.f } };
	const vk::ImageSubresourceRange range{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

	cmd.clearColorImage(image, vk::ImageLayout::eTransferDstOptimal, color, range);

	TransitionImageLayout(cmd, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eColorAttachmentOptimal, image);
}
