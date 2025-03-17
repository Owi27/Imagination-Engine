#include "pch.h"
#include "VulkanResources.h"

void Texture::CreateImage(const VkExtent3D& extent, const VkSampleCountFlagBits& msaaBit, const VkFormat& format, const VkImageTiling& tiling, const VkImageUsageFlags& usageFlags, const VkMemoryPropertyFlags& memoryPropertyFlags, VkAllocationCallbacks* allocator)
{
	_extent = extent;
	_sampleCountFlagBits = msaaBit;
	_format = format;
	_imageTiling = tiling;
	_imageUsageFlags = usageFlags;
	_imageMemoryPropertyFlags = memoryPropertyFlags;

	GvkHelper::create_image(_vk->GetPhysicalDevice(), _vk->GetDevice(), _extent, _mipLevels, _sampleCountFlagBits, _format, _imageTiling, _imageUsageFlags, _imageMemoryPropertyFlags, allocator, &_image, &_imageMemory);
}

void Texture::CreateImageView(const VkImageAspectFlags& imageAspectFlags, VkAllocationCallbacks* allocator)
{
	_imageAspectFlags = imageAspectFlags;
	
	GvkHelper::create_image_view(_vk->GetDevice(), _image, _format, _imageAspectFlags, _mipLevels, allocator, &_imageView);
}

void Buffer::CreateBuffer(const VkDeviceSize& size, const VkBufferUsageFlags& bufferUsageFlags, const VkMemoryPropertyFlags& memoryPropertyFlags)
{
	_size = size;
	_bufferUsageFlags = bufferUsageFlags;
	_bufferMemoryPropertyFlags = memoryPropertyFlags;
	
	GvkHelper::create_buffer(_vk->GetPhysicalDevice(), _vk->GetDevice(), _size, _bufferUsageFlags, _bufferMemoryPropertyFlags, &_buffer, &_bufferMemory);
}

void Buffer::WriteToBuffer(const void* dataToWrite)
{
	GvkHelper::write_to_buffer(_vk->GetDevice(), _bufferMemory, dataToWrite, _size);
}
