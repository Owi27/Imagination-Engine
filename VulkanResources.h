#pragma once

class VulkanResource
{

protected:
	std::string _name;
	std::unordered_set<std::string> _writtenInPasses;
	std::unordered_set<std::string> _readInPasses;
	VulkanContext& _vk;

public:
	VulkanResource() : _vk(*VulkanContext::GetInst())
	{

	}

	~VulkanResource()
	{

	}

		std::string& GetName() { return _name; }

	void SetName(const std::string& name) { _name = name; }
	void WrittenInPass(const std::string& pass) { _writtenInPasses.insert(pass); }
	void ReadInPass(const std::string& pass) { _readInPasses.insert(pass); }
};

class Buffer : public VulkanResource
{
	VkBuffer _buffer;
	VkDeviceMemory _bufferMemory;
	VkDeviceSize _size;
	VkBufferUsageFlags _bufferUsageFlags;
	VkMemoryPropertyFlags _bufferMemoryPropertyFlags;

public:
	Buffer()
	{

	}

	~Buffer()
	{
		vkDestroyBuffer(_vk.GetDevice(), _buffer, nullptr);
		vkFreeMemory(_vk.GetDevice(), _bufferMemory, nullptr);
	}

	VkBuffer& GetBuffer() { return _buffer; }
	VkDeviceMemory& GetMemory() { return _bufferMemory; }

	void CreateBuffer(const VkDeviceSize& size, const VkBufferUsageFlags& bufferUsageFlags, const VkMemoryPropertyFlags& memoryPropertyFlags);
	void WriteToBuffer(const void* data);

	bool operator==(const Buffer& other) const
	{
		return (_name == other._name) &&
			(_buffer == other._buffer) &&
			(_bufferMemory == other._bufferMemory) &&
			(_size == other._size) &&
			(_bufferUsageFlags == other._bufferUsageFlags) &&
			(_bufferMemoryPropertyFlags == other._bufferMemoryPropertyFlags);
	}
};

class Texture : public VulkanResource
{
	VkImage _image;
	VkImageView _imageView;
	VkSampler _sampler;
	VkDeviceMemory _imageMemory;
	VkImageTiling _imageTiling;
	VkSampleCountFlagBits _sampleCountFlagBits;
	VkImageAspectFlags _imageAspectFlags;
	VkExtent3D _extent;
	VkFormat _format;
	unsigned _mipLevels = 0;
	VkImageUsageFlags _imageUsageFlags;
	VkMemoryPropertyFlags _imageMemoryPropertyFlags;

	VkAttachmentDescription _attachmentDescription;

	VkClearColorValue _clearColorValue;

public:
	Texture()
	{

	}

	~Texture()
	{
		vkDestroyImage(_vk.GetDevice(), _image, nullptr);
		vkDestroyImageView(_vk.GetDevice(), _imageView, nullptr);
		vkFreeMemory(_vk.GetDevice(), _imageMemory, nullptr);
	}

	VkImage& GetImage() { return _image; }
	VkImageView& GetImageView() { return _imageView; }
	VkExtent3D& GetExtent() { return _extent; }
	VkClearColorValue& GetClearColorValue() { return _clearColorValue; }

	void SetClearColorValue(const VkClearColorValue& clearColorValue) { _clearColorValue = clearColorValue; }

	void CreateImage(const VkExtent3D& extent, const VkSampleCountFlagBits& msaaBit, const VkFormat& format, const VkImageTiling& tiling, const VkImageUsageFlags& usageFlags, const VkMemoryPropertyFlags& memoryPropertyFlags, VkAllocationCallbacks* allocator = nullptr);
	void CreateImageView(const VkImageAspectFlags& imageAspectFlags, VkAllocationCallbacks* allocator = nullptr);

	bool operator==(const Texture& other) const
	{
		return (_name == other._name) &&
			(_image == other._image) &&
			(_imageView == other._imageView) &&
			(_imageMemory == other._imageMemory) &&
			(_imageTiling == other._imageTiling) &&
			(_sampleCountFlagBits == other._sampleCountFlagBits) &&
			(_imageAspectFlags == other._imageAspectFlags) &&
			(_extent.width == other._extent.width) &&
			(_extent.height == other._extent.height) &&
			(_extent.depth == other._extent.depth) &&
			(_format == other._format) &&
			(_mipLevels == other._mipLevels) &&
			(_imageUsageFlags == other._imageUsageFlags) &&
			(_imageMemoryPropertyFlags == other._imageMemoryPropertyFlags) &&
			(_attachmentDescription.flags == other._attachmentDescription.flags) &&
			(_clearColorValue.float32[0] == other._clearColorValue.float32[0]) &&  // Clear color comparison
			(_clearColorValue.float32[1] == other._clearColorValue.float32[1]) &&
			(_clearColorValue.float32[2] == other._clearColorValue.float32[2]) &&
			(_clearColorValue.float32[3] == other._clearColorValue.float32[3]);
	}
};