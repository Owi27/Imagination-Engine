#pragma once

namespace ImgnVulkan
{
	struct IMGN_VULKAN_API Buffer
	{
		vk::raii::Buffer buffer = nullptr;
		vk::raii::DeviceMemory memory = nullptr;
	};

	struct IMGN_VULKAN_API Image
	{
		vk::raii::Image image = nullptr;
		vk::raii::ImageView imageView = nullptr;
		vk::raii::DeviceMemory memory = nullptr;
	};

	struct IMGN_VULKAN_API Pipelines
	{
		vk::raii::Pipeline gBufferPipeline = nullptr, lightingPipeline = nullptr, generalPipeline = nullptr;
		vk::raii::PipelineLayout pipelineLayout = nullptr;
	};
}