#include "pch.h"
#include "VulkanResources.h"

void Texture::SetImageLayout(VkImageLayout newLayout)
{
	if (_imageLayout == newLayout) return;

	GvkHelper::transition_image_layout(_vk.GetDevice(), _vk.GetCommandPool(), _vk.GetGraphicsQueue(), VK_REMAINING_MIP_LEVELS, _image, _format, _imageLayout, newLayout);
	_imageLayout = newLayout;
}

void Texture::SetImageLayout(VkCommandBuffer& commandBuffer, VkImageLayout newLayout)
{
	if (_imageLayout == newLayout) return;

	GvkHelper::transition_image_layout(commandBuffer,_vk.GetDevice(), _vk.GetCommandPool(), _vk.GetGraphicsQueue(), VK_REMAINING_MIP_LEVELS, _image, _format, _imageLayout, newLayout);
	_imageLayout = newLayout;
}

void Texture::TransitionLayout(VkCommandBuffer& commandBuffer)
{
	_vk.TransitionImageLayout(commandBuffer, VK_REMAINING_MIP_LEVELS, _image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR);
}

void Texture::TransitionLayout()
{
	VkCommandBuffer commandBuffer;
	GvkHelper::signal_command_start(_vk.GetDevice(), _vk.GetCommandPool(), &commandBuffer);
	_vk.TransitionImageLayout(commandBuffer, VK_REMAINING_MIP_LEVELS, _image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR);
	GvkHelper::signal_command_end(_vk.GetDevice(), _vk.GetGraphicsQueue(), _vk.GetCommandPool(), &commandBuffer);
}

void Texture::CreateImage(const VkExtent3D& extent, const VkSampleCountFlagBits& msaaBit, const VkFormat& format, const VkImageTiling& tiling, const VkImageUsageFlags& usageFlags, const VkMemoryPropertyFlags& memoryPropertyFlags, VkAllocationCallbacks* allocator)
{
	_extent = extent;
	_sampleCountFlagBits = msaaBit;
	_format = format;
	_imageTiling = tiling;
	_imageUsageFlags = usageFlags;
	_imageMemoryPropertyFlags = memoryPropertyFlags;

	GvkHelper::create_image(_vk.GetPhysicalDevice(), _vk.GetDevice(), _extent, 1, _sampleCountFlagBits, _format, _imageTiling, _imageUsageFlags, _imageMemoryPropertyFlags, allocator, &_image, &_imageMemory);
}

void Texture::CreateImageView(const VkImageAspectFlags& imageAspectFlags, VkAllocationCallbacks* allocator)
{
	_imageAspectFlags = imageAspectFlags;
	
	GvkHelper::create_image_view(_vk.GetDevice(), _image, _format, _imageAspectFlags, 1, allocator, &_imageView);
}

void Buffer::CreateBuffer(const VkDeviceSize& size, const VkBufferUsageFlags& bufferUsageFlags, const VkMemoryPropertyFlags& memoryPropertyFlags)
{
	_size = size;
	_bufferUsageFlags = bufferUsageFlags;
	_bufferMemoryPropertyFlags = memoryPropertyFlags;
	
	GvkHelper::create_buffer(_vk.GetPhysicalDevice(), _vk.GetDevice(), _size, _bufferUsageFlags, _bufferMemoryPropertyFlags, &_buffer, &_bufferMemory);
}

void Buffer::WriteToBuffer(const void* dataToWrite)
{
	//_data = dataToWrite;
	GvkHelper::write_to_buffer(_vk.GetDevice(), _bufferMemory, dataToWrite, _size);
}

void Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mappedMemoryRange
	{
		.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		.memory = _bufferMemory,
		.offset = offset,
		.size = size,
	};

	vkFlushMappedMemoryRanges(_vk.GetDevice(), 1, &mappedMemoryRange);
}
