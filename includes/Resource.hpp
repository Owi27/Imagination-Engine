#pragma once

struct IResource
{
	VkDeviceMemory memory;

	IResource() = default;
	IResource(VulkanContext pCtx)
	{
		_vk = pCtx;
	}

protected:
	VulkanContext _vk;

	unsigned FindMemoryType(unsigned pFilter, MemoryFlags pMemoryFlags);
};

class Buffer : IResource
{
public:
	VkBuffer buffer;
	VkDeviceSize size;

	Buffer() = default;
	Buffer(VulkanContext pCtx) : IResource(pCtx)
	{

	}

	~Buffer()
	{
		vkDestroyBuffer(_vk.device, buffer, nullptr);
		vkFreeMemory(_vk.device, memory, nullptr);
	}

	void CopyBuffer(Buffer pSourceBuffer);
	Buffer& CreateBuffer(VkDeviceSize pSize, BufferUsage pUsage, MemoryFlags pMemory);
	void WriteToBuffer(const void* pData);
};

class Texture : IResource //texture, image same shii
{
	unsigned _mipLevels;
	unsigned _width, _height;
	VkExtent3D _extent;

public:
	VkImage image;
	VkImageView imageView;
	PipelineFormat format;

	Texture() = default;
	Texture(VulkanContext pCtx) : IResource(pCtx)
	{

	}

	~Texture()
	{
		vkDestroyImage(_vk.device, image, nullptr);
		vkDestroyImageView(_vk.device, imageView, nullptr);
		vkFreeMemory(_vk.device, memory, nullptr);
	}

	Texture& LoadImage(const std::string& pImgPath);
	Texture& CreateImage(VkExtent3D pExtent, unsigned pMipLevels, SampleCount pSampleCount, PipelineFormat pFormat, ImageTiling pImageTiling, ImageUsage pUsage, MemoryFlags pMemory);
	void CreateImageView(ImageAspect pImageAspect);
	Texture& CopyBuffer(Buffer pSourceBuffer);
};