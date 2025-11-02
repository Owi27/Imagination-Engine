#include "D:/GitHub/Imagination-Engine/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"

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

	vkCreateBuffer(_device, &bufferCreateInfo, nullptr, &buffer);

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(_device, buffer, &memoryRequirements);
	VkMemoryAllocateInfo memoryAllocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		//.pNext = ,
		.allocationSize = memoryRequirements.size,
		//.memoryTypeIndex = ,
	};

	memoryAllocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, pMemory);

	vkAllocateMemory(_device, &memoryAllocateInfo, nullptr, &memory);
	vkBindBufferMemory(_device, buffer, memory, 0);

	return *this;
}

void Buffer::WriteToBuffer(const void* pData)
{
	void* data;
	vkMapMemory(_device, memory, 0, size, 0, &data);
	memcpy(data, pData, size);
	vkUnmapMemory(_device, memory);
}

unsigned IResource::FindMemoryType(unsigned pFilter, MemoryFlags pMemoryFlags)
{
	unsigned out;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memoryProperties);

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

	vkCreateImage(_device, &imageCreateInfo, nullptr, &image);

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(_device, image, &memoryRequirements);
	VkMemoryAllocateInfo memoryAllocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		//.pNext = ,
		.allocationSize = memoryRequirements.size,
		//.memoryTypeIndex = ,
	};

	memoryAllocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, pMemory);

	vkAllocateMemory(_device, &memoryAllocateInfo, nullptr, &memory);
	vkBindImageMemory(_device, image, memory, 0);

	format = pFormat;
	_mipLevels = pMipLevels;

	return *this;
}

void Texture::CreateImageView(ImageAspect pImageAspect)
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

	vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &imageView);
}
