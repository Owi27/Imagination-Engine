#pragma once

class VulkanResource
{
	std::string _name;
	std::unordered_set<std::string> _writtenInPasses;
	std::unordered_set<std::string> _readInPasses;

public:
	VulkanResource()
	{

	}

	~VulkanResource()
	{

	}

	void SetName(std::string& name) { _name = name; }
	void WrittenInPass(std::string& pass) { _writtenInPasses.insert(pass); }
	void ReadInPass(std::string& pass) { _readInPasses.insert(pass); }

};

class Buffer : public VulkanResource
{
	std::shared_ptr<VulkanContext> _vk;

	VkBuffer _buffer;
	VkDeviceMemory _bufferMemory;
	VkDeviceSize _size;
	VkBufferUsageFlags _bufferUsageFlags;
	VkMemoryPropertyFlags _bufferMemoryPropertyFlags;

public:
	Buffer(VulkanContext& vkContext)
	{
		_vk = std::make_shared<VulkanContext>(vkContext);
	}

	~Buffer()
	{
		vkDestroyBuffer(_vk->GetDevice(), _buffer, nullptr);
		vkFreeMemory(_vk->GetDevice(), _bufferMemory, nullptr);
	}

	VkBuffer& GetBuffer() { return _buffer; }
	VkDeviceMemory& GetMemory() { return _bufferMemory; }

	void CreateBuffer(const VkDeviceSize& size, const VkBufferUsageFlags& bufferUsageFlags, const VkMemoryPropertyFlags& memoryPropertyFlags);
	void WriteToBuffer(const void* data);
};

class Texture : public VulkanResource
{
	std::shared_ptr<VulkanContext> _vk;

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

public:
	Texture(VulkanContext& vkContext)
	{
		_vk = std::make_shared<VulkanContext>(vkContext);
	}

	~Texture()
	{
		vkDestroyImage(_vk->GetDevice(), _image, nullptr);
		vkDestroyImageView(_vk->GetDevice(), _imageView, nullptr);
		vkFreeMemory(_vk->GetDevice(), _imageMemory, nullptr);
	}

	VkImage& GetImage() { return _image; }
	VkImageView& GetImageView() { return _imageView; }
	VkExtent3D& GetExtent() { return _extent; }

	void CreateImage(const VkExtent3D& extent, const VkSampleCountFlagBits& msaaBit, const VkFormat& format, const VkImageTiling& tiling, const VkImageUsageFlags& usageFlags, const VkMemoryPropertyFlags& memoryPropertyFlags, VkAllocationCallbacks* allocator = nullptr);
	void CreateImageView(const VkImageAspectFlags& imageAspectFlags, VkAllocationCallbacks* allocator = nullptr);
};