#include "pch.h"
#include "VkResources.h"
#include "VulkanCtx.h"
#include "gltf/tiny_gltf.h"
#undef LoadImage

unsigned IResource::FindMemoryType(unsigned pFilter, MemoryFlags pMemoryFlags)
{
	unsigned out;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(VkCtx::Instance().physicalDevice, &memoryProperties);

	for (size_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((pFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & (VkMemoryPropertyFlags)pMemoryFlags) == (VkMemoryPropertyFlags)pMemoryFlags)
		{
			out = i;
			return out;
		}
	}

	return -1;
}

Buffer& Buffer::CreateBuffer(VkDeviceSize pSize, BufferUsage pUsage, MemoryFlags pMemory)
{
	size = pSize;

	VkBufferCreateInfo bufferCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		//.pNext = ,
		//.flags = ,
		.size = size,
		.usage = (VkBufferUsageFlags)pUsage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		//.queueFamilyIndexCount = ,
		//.pQueueFamilyIndices = ,
	};

	vkCreateBuffer(VkCtx::Instance().device, &bufferCreateInfo, nullptr, &buffer);

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(VkCtx::Instance().device, buffer, &memoryRequirements);
	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
		//.pNext = ,
		.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
		//.deviceMask = ,
	};

	VkMemoryAllocateInfo memoryAllocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = (pUsage & BufferUsage::DEVICE_ADDRESS) == BufferUsage::DEVICE_ADDRESS ? &memoryAllocateFlagsInfo : nullptr,
		.allocationSize = memoryRequirements.size,
		//.memoryTypeIndex = ,
	};

	memoryAllocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, pMemory);

	vkAllocateMemory(VkCtx::Instance().device, &memoryAllocateInfo, nullptr, &memory);
	vkBindBufferMemory(VkCtx::Instance().device, buffer, memory, 0);

	return *this;
}

void Buffer::WriteToBuffer(const void* pData)
{
	void* data;
	vkMapMemory(VkCtx::Instance().device, memory, 0, size, 0, &data);
	memcpy(data, pData, size);
	vkUnmapMemory(VkCtx::Instance().device, memory);
}

void Buffer::CopyBuffer(Buffer pSourceBuffer)
{
	VkCommandBuffer commandBuffer = VkHelpers::StartCommandBuffer();

	VkBufferCopy bufferCopy
	{
		.srcOffset = 0,
		.dstOffset = 0,
		.size = pSourceBuffer.size,
	};

	vkCmdCopyBuffer(commandBuffer, pSourceBuffer.buffer, buffer, 1, &bufferCopy);

	VkHelpers::EndCommandBuffer(commandBuffer);
}

uint64_t Buffer::GetDeviceAddress()
{
	VkBufferDeviceAddressInfo deviceAddressInfo
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		//.pNext = ,
		.buffer = buffer,
	};

	return vkGetBufferDeviceAddress(VkCtx::Instance().device, &deviceAddressInfo);
}

VkAccessFlags Texture::GetAccessFlag(ImageLayout pImageLayout)
{
	switch (pImageLayout)
	{
	case ImageLayout::GENERAL:
		return VK_ACCESS_NONE;
	case ImageLayout::COLOR_ATTACHMENT:
		return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	case ImageLayout::DEPTH_STENCIL:
		return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	case ImageLayout::DEPTH_STENCIL_READ:
		return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	case ImageLayout::SHADER_READ:
		return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	case ImageLayout::SOURCE:
		return VK_ACCESS_TRANSFER_READ_BIT;
	case ImageLayout::DESTINATION:
		return VK_ACCESS_TRANSFER_WRITE_BIT;
	case ImageLayout::PRESENT_SRC:
		return VK_ACCESS_NONE;
	case ImageLayout::UNDEFINED:
	default:
		return VK_ACCESS_NONE;
	}
}

VkPipelineStageFlags Texture::GetPipelineStageFlags(ImageLayout pImageLayout)
{
	switch (pImageLayout)
	{
	case ImageLayout::GENERAL:
		return VK_PIPELINE_STAGE_NONE;
	case ImageLayout::COLOR_ATTACHMENT:
		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	case ImageLayout::DEPTH_STENCIL:
		return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	case ImageLayout::DEPTH_STENCIL_READ:
		return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	case ImageLayout::SHADER_READ:
		return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	case ImageLayout::SOURCE:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	case ImageLayout::DESTINATION:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	case ImageLayout::PRESENT_SRC:
		return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	case ImageLayout::UNDEFINED:
		return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	default:
		return VK_PIPELINE_STAGE_NONE;
	}
}

Texture& Texture::CreateImage(VkExtent3D pExtent, unsigned pMipLevels, SampleCount pSampleCount, PipelineFormat pFormat, ImageTiling pImageTiling, ImageUsage pUsage, MemoryFlags pMemory)
{
	VkImageCreateInfo imageCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		//.pNext = ,
		//.flags = ,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = (VkFormat)pFormat,
		.extent = (VkExtent3D)pExtent,
		.mipLevels = pMipLevels,
		.arrayLayers = 1,
		.samples = (VkSampleCountFlagBits)pSampleCount,
		.tiling = (VkImageTiling)pImageTiling,
		.usage = (VkImageUsageFlags)pUsage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		//.queueFamilyIndexCount = ,
		//.pQueueFamilyIndices = ,
		.initialLayout = (VkImageLayout)ImageLayout::UNDEFINED,
	};

	vkCreateImage(VkCtx::Instance().device, &imageCreateInfo, nullptr, &image);

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(VkCtx::Instance().device, image, &memoryRequirements);
	VkMemoryAllocateInfo memoryAllocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		//.pNext = ,
		.allocationSize = memoryRequirements.size,
		//.memoryTypeIndex = ,
	};

	memoryAllocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, pMemory);

	vkAllocateMemory(VkCtx::Instance().device, &memoryAllocateInfo, nullptr, &memory);
	vkBindImageMemory(VkCtx::Instance().device, image, memory, 0);

	format = pFormat;
	imageLayout = ImageLayout::UNDEFINED;
	_mipLevels = pMipLevels;
	_extent = pExtent;
	_width = pExtent.width;
	_height = pExtent.height;

	return *this;
}

Texture& Texture::CreateImageView(ImageAspect pImageAspect)
{
	VkImageViewCreateInfo imageViewCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		//.pNext = ,
		//.flags = ,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = (VkFormat)format,
		.components
		{
			.r = VK_COMPONENT_SWIZZLE_R,
			.g = VK_COMPONENT_SWIZZLE_G,
			.b = VK_COMPONENT_SWIZZLE_B,
			.a = VK_COMPONENT_SWIZZLE_A,
		},
		.subresourceRange
		{
			.aspectMask = (VkImageAspectFlags)pImageAspect,
			.baseMipLevel = 0,
			.levelCount = _mipLevels,
			.baseArrayLayer = 0,
			.layerCount = 1,
		}
	};

	vkCreateImageView(VkCtx::Instance().device, &imageViewCreateInfo, nullptr, &imageView);

	_imageAspect = pImageAspect;

	return *this;
}

Texture& Texture::CopyBuffer(Buffer& pSourceBuffer)
{
	VkCommandBuffer commandBuffer = VkHelpers::StartCommandBuffer();

	VkBufferImageCopy bufferImageCopy
	{
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource
		{
			.aspectMask = (VkImageAspectFlags)ImageAspect::COLOR,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageOffset = { 0, 0, 0 },
		.imageExtent = _extent,
	};

	vkCmdCopyBufferToImage(commandBuffer, pSourceBuffer.buffer, image, (VkImageLayout)ImageLayout::DESTINATION, 1, &bufferImageCopy);

	VkHelpers::EndCommandBuffer(commandBuffer);

	return *this;
}

Texture& Texture::LoadImage(const std::string& pImgPath)
{
	int width, height, component;

	auto data = stbi_load(pImgPath.c_str(), &width, &height, &component, STBI_rgb_alpha);

	int imgSize = width * height * 4;
	format = PipelineFormat::COLOR;

	Buffer staging;
	staging.CreateBuffer(imgSize, BufferUsage::SOURCE, MemoryFlags::CPU | MemoryFlags::CPU2GPU);
	staging.WriteToBuffer(data);

	unsigned mipLevels = static_cast<unsigned>(floor(log2(std::max(width, height))) + 1);

	CreateImage({ static_cast<unsigned>(width), static_cast<unsigned>(height), 1 }, mipLevels, SampleCount::SAMPLE_1BIT, format, ImageTiling::OPTIMAL, ImageUsage::SOURCE | ImageUsage::DESTINATION | ImageUsage::SAMPLED, MemoryFlags::GPU).CreateImageView(ImageAspect::COLOR).TransitionImageLayout(ImageLayout::DESTINATION).CopyBuffer(staging);
	stbi_image_free(data);

	return *this;
}

Texture& Texture::TransitionImageLayout(ImageLayout pNewLayout)
{
	VkCommandBuffer commandBuffer = VkHelpers::StartCommandBuffer();

	VkImageMemoryBarrier imageMemoryBarrier
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		//.pNext = ,
		.srcAccessMask = GetAccessFlag(imageLayout),
		.dstAccessMask = GetAccessFlag(pNewLayout),
		.oldLayout = (VkImageLayout)imageLayout,
		.newLayout = (VkImageLayout)pNewLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange
		{
			.aspectMask = (VkImageAspectFlags)_imageAspect,
			.baseMipLevel = 0,
			.levelCount = _mipLevels,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};

	vkCmdPipelineBarrier(commandBuffer, GetPipelineStageFlags(imageLayout), GetPipelineStageFlags(pNewLayout), 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	VkHelpers::EndCommandBuffer(commandBuffer);

	imageLayout = pNewLayout;
	return *this;
}
