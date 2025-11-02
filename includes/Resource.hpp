#pragma once

struct IResource
{
	VkDeviceMemory memory;

	IResource() = default;
	IResource(VkDevice pDevice, VkPhysicalDevice pPhysicalDevice)
	{
		_device = pDevice;
		_physicalDevice = pPhysicalDevice;
	}

protected:
	VkDevice _device;
	VkPhysicalDevice _physicalDevice;

	unsigned FindMemoryType(unsigned pFilter, MemoryFlags pMemoryFlags);
};

class Buffer : IResource
{
public:
	VkBuffer buffer;
	VkDeviceSize size;

	Buffer() = default;
	Buffer(VkDevice pDevice, VkPhysicalDevice pPhysicalDevice) : IResource(pDevice, pPhysicalDevice)
	{

	}

	~Buffer()
	{

	}

	Buffer& CreateBuffer(VkDeviceSize pSize, BufferUsage pUsage, MemoryFlags pMemory);
	void WriteToBuffer(const void* pData);
};

class Texture : IResource //texture, image same shii
{
	unsigned _mipLevels;

public:
	VkImage image;
	VkImageView imageView;
	PipelineFormat format;

	Texture() = default;
	Texture(VkDevice pDevice, VkPhysicalDevice pPhysicalDevice) : IResource(pDevice, pPhysicalDevice)
	{

	}

	~Texture()
	{

	}

	Texture& CreateImage(VkExtent3D pExtent, unsigned pMipLevels, SampleCount pSampleCount, PipelineFormat pFormat, ImageTiling pImageTiling, ImageUsage pUsage, MemoryFlags pMemory);
	void CreateImageView(ImageAspect pImageAspect);
};