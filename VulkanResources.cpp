#include "pch.h"
#include "VulkanResources.h"

void Texture::MakeMipLevels()
{
	GvkHelper::create_mipmaps(_vk.get()->GetDevice(), _vk.get()->GetCommandPool(), _vk.get()->GetGraphicsQueue(), _image, _extent.width, _extent.height, _mipLevels);
}

void Texture::CreateTexture()
{
	GvkHelper::create_image(_vk.get()->GetPhysicalDevice(), _vk.get()->GetDevice(), _extent, _mipLevels, VK_SAMPLE_COUNT_4_BIT, _format, VK_IMAGE_TILING_OPTIMAL, _imageUsageFlags, _imageMemoryPropertyFlags, nullptr, &_image, &_imageMemory);
}
