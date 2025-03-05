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
	VkBuffer _buffer;
	VkDeviceMemory _bufferMemory;

public:
	Buffer()
	{

	}

	~Buffer()
	{

	}

	VkBuffer& GetVkBuffer() { return _buffer; }
	VkDeviceMemory& GetVkMemory() { return _bufferMemory; }
};

class Texture : public VulkanResource
{
	VkImage _image;
	VkImageView _imageView;
	VkSampler _sampler;
	VkDeviceMemory _imageMemory;

	std::shared_ptr<VulkanContext> _vk;

	VkExtent3D _extent;
	VkFormat _format;
	unsigned _mipLevels = 0;
	VkImageUsageFlags _imageUsageFlags;
	VkMemoryPropertyFlags _imageMemoryPropertyFlags;
	VkAttachmentDescription _attachmentDescription;

	void MakeMipLevels();

public:
	Texture(VulkanContext& vkContext)
	{
		_vk = std::make_shared<VulkanContext>(vkContext);
	}

	~Texture()
	{

	}

	VkImage& GetVkImage() { return _image; }
	VkExtent3D& GetExtent() { return _extent; }

	void AddUsageFlags(VkImageUsageFlags& usageFlags) { _imageUsageFlags |= usageFlags; }
	void AddMemoryFlags(VkMemoryPropertyFlags& memoryPropertyFlags) { _imageMemoryPropertyFlags |= memoryPropertyFlags; }
	void SetFormat(VkFormat& format) { _format = format; }
	void SetExtent(unsigned width, unsigned height, unsigned depth = 1) { _extent = { width, height, depth }; }

	void CreateTexture();
};