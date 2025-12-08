#pragma once
#undef LoadImage


struct IResource
{
	VkDeviceMemory memory;

	IResource() = default;
	IResource(VulkanContext& pCtx)
	{
		_vk = std::make_shared<VulkanContext>(pCtx);
	}

protected:
	std::shared_ptr<VulkanContext> _vk;

	unsigned FindMemoryType(unsigned pFilter, MemoryFlags pMemoryFlags);
};

class Buffer : IResource
{
public:
	VkBuffer buffer;
	VkDeviceSize size;

	Buffer() = default;
	Buffer(VulkanContext& pCtx) : IResource(pCtx)
	{
	}

	~Buffer()
	{
		if (buffer) vkDestroyBuffer(_vk->device, buffer, nullptr);
		if (memory) vkFreeMemory(_vk->device, memory, nullptr);
	}

	Buffer(const Buffer& pOther) = delete;
	Buffer& operator=(const Buffer& pOther) = delete;

	Buffer(Buffer&& pOther) noexcept : IResource(std::move(pOther))
	{
		buffer = pOther.buffer;
		memory = pOther.memory;
		size = pOther.size;

		pOther.buffer = nullptr;
		pOther.memory = nullptr;
		pOther.size = 0;
	}

	Buffer& operator=(Buffer&& pOther) noexcept
	{
		if (this != &pOther)
		{
			//delete this;

			IResource::operator=(std::move(pOther));

			buffer = pOther.buffer;
			memory = pOther.memory;
			size = pOther.size;

			pOther.buffer = nullptr;
			pOther.memory = nullptr;
			pOther.size = 0;
		}

		return *this;
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
	ImageAspect _imageAspect;

	VkAccessFlags GetAccessFlag(ImageLayout pImageLayout);
	VkPipelineStageFlags GetPipelineStageFlags(ImageLayout pImageLayout);

public:
	VkImage image;
	VkImageView imageView;
	PipelineFormat format;
	ImageLayout imageLayout;
	bool owns = true;

	Texture() = default;
	Texture(VulkanContext& pCtx) : IResource(pCtx)
	{

	}

	~Texture()
	{
		if (image && owns) vkDestroyImage(_vk->device, image, nullptr);
		if (imageView && owns) vkDestroyImageView(_vk->device, imageView, nullptr);
		if (memory && owns) vkFreeMemory(_vk->device, memory, nullptr);
	}

	Texture(const Texture& pOther) = delete;
	Texture& operator=(const Texture& pOther) = delete;

	Texture(Texture&& pOther) noexcept : IResource(std::move(pOther))
	{	
		image = pOther.image;
		imageView = pOther.imageView;
		memory = pOther.memory;
		format = pOther.format;
		_mipLevels = pOther._mipLevels;
		_width = pOther._width;
		_height = pOther._height;
		_extent = pOther._extent;
		imageLayout = pOther.imageLayout;

		pOther.image = nullptr;
		pOther.imageView = nullptr;
		pOther.memory = nullptr;
		pOther.format = PipelineFormat::UNDEFINED;
		pOther._mipLevels = 0;
		pOther._width = 0;
		pOther._height = 0;
		pOther._extent = {};
		pOther.imageLayout = ImageLayout::UNDEFINED;
	}

	Texture& operator=(Texture&& pOther) noexcept
	{
		if (this != &pOther) 
		{
			//delete this;
			IResource::operator=(std::move(pOther));

			image = pOther.image;
			imageView = pOther.imageView;
			memory = pOther.memory;
			format = pOther.format;
			_mipLevels = pOther._mipLevels;
			_width = pOther._width;
			_height = pOther._height;
			_extent = pOther._extent;
			imageLayout = pOther.imageLayout;

			pOther.image = nullptr;
			pOther.imageView = nullptr;
			pOther.memory = nullptr;
			pOther.format = PipelineFormat::UNDEFINED;
			pOther._mipLevels = 0;
			pOther._width = 0;
			pOther._height = 0;
			pOther._extent = {};
			pOther.imageLayout = ImageLayout::UNDEFINED;
		}

		return *this;
	}

	Texture& Swapchain(VkImage pImage, VkImageView pImageView, PipelineFormat pFormat, VkExtent3D pExtent, ImageAspect pImageAspect);
	Texture& LoadImage(const std::string& pImgPath);
	Texture& CreateImage(VkExtent3D pExtent, unsigned pMipLevels, SampleCount pSampleCount, PipelineFormat pFormat, ImageTiling pImageTiling, ImageUsage pUsage, MemoryFlags pMemory);
	Texture& CreateImageView(ImageAspect pImageAspect);
	Texture& CopyBuffer(Buffer& pSourceBuffer);
	Texture& TransitionImageLayout(ImageLayout pNewLayout);
	Texture& SetOwnership(bool pOwns);
};