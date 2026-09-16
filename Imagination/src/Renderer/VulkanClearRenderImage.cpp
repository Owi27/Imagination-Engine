#include "pch.hpp"
#include "Vulkan.hpp"

void Vulkan::ClearRenderImage(RGImage& pImage)
{
	// Zero a newly created color RG image so first-frame history is defined
	// even if historyValid is wrong. Uses Transfer clear + ShaderReadOnly.
	if (pImage.aspect & vk::ImageAspectFlagBits::eDepth)
		return;

	unique<vk::raii::CommandBuffer> cmd = StartSingleTimeCommand();

	TransitionImageLayout(*cmd, pImage.currentLayout, vk::ImageLayout::eTransferDstOptimal, *pImage.image.image, pImage.aspect);
	pImage.currentLayout = vk::ImageLayout::eTransferDstOptimal;

	vk::ClearColorValue clearColor{ std::array<float, 4>{ 0.f, 0.f, 0.f, 0.f } };
	vk::ImageSubresourceRange range{ pImage.aspect, 0, 1, 0, 1 };
	cmd->clearColorImage(*pImage.image.image, vk::ImageLayout::eTransferDstOptimal, clearColor, range);

	TransitionImageLayout(*cmd, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, *pImage.image.image, pImage.aspect);
	pImage.currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	EndSingleTimeCommand(*cmd);
}
