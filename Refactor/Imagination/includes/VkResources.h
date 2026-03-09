#pragma once
#undef LoadImage

struct IResource
{
	VkDeviceMemory memory;

protected:
	unsigned FindMemoryType(unsigned pFilter, MemoryFlags pMemoryFlags);
};

class Buffer;

class Texture : public IResource //texture, image, same shii
{
	unsigned _mipLevels = 1;
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

	Texture() : IResource() /*Constructor*/
	{
	}

	~Texture() /*Destructor*/
	{
	}

	Texture(const Texture& pOther) = delete;

	Texture& operator=(const Texture& pOther) = delete;

	Texture(Texture&& pOther) noexcept : IResource() /*Move Constructor*/
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

	Texture& operator=(Texture&& pOther) noexcept /*Move Assignment Operator*/
	{
		if (this != &pOther)
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

		return *this;
	}

	Texture& CreateImage(VkExtent3D pExtent, unsigned pMipLevels, SampleCount pSampleCount, PipelineFormat pFormat, ImageTiling pImageTiling, ImageUsage pUsage, MemoryFlags pMemory);
	Texture& CreateImageView(ImageAspect pImageAspect);
	Texture& CopyBuffer(Buffer& pSourceBuffer);
	Texture& LoadImage(const std::string& pImgPath);
	Texture& LoadImage(VkExtent3D pExtent, unsigned pComponent, void* pData);
	Texture& TransitionImageLayout(ImageLayout pNewLayout);

};

class Buffer : public IResource
{
	void* _data = nullptr;

public:
	VkBuffer buffer;
	VkDeviceSize size;

	Buffer() : IResource() /*Constructor*/
	{
	}

	~Buffer() /*Destructor*/
	{
	}

	Buffer(const Buffer& pOther) = delete;

	Buffer& operator=(const Buffer& pOther) = delete;

	Buffer(Buffer&& pOther) noexcept : IResource() /*Move Constructor*/
	{
		buffer = pOther.buffer;
		memory = pOther.memory;
		size = pOther.size;

		pOther.buffer = nullptr;
		pOther.memory = nullptr;
		pOther.size = 0;
	}

	Buffer& operator=(Buffer&& pOther) noexcept /*Move Assignment Operator*/
	{
		if (this != &pOther)
		{
			buffer = pOther.buffer;
			memory = pOther.memory;
			size = pOther.size;

			pOther.buffer = nullptr;
			pOther.memory = nullptr;
			pOther.size = 0;
		}

		return *this;
	}

	Buffer& CreateBuffer(VkDeviceSize pSize, BufferUsage pUsage, MemoryFlags pMemory);
	Buffer& CreateMappedBuffer(VkDeviceSize pSize, BufferUsage pUsage, MemoryFlags pMemory);
	void WriteToBuffer(void* pData);
	void CopyBuffer(Buffer pSourceBuffer);
	uint64_t GetDeviceAddress();

	void* GetData() { return _data; }

};