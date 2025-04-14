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

	Buffer(VkDeviceSize size, void* data, VkBufferUsageFlags usage, bool nullMemory = false)
	{
		_size = size;
		_bufferUsageFlags = usage;
		_bufferMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		GvkHelper::create_buffer(_vk.GetPhysicalDevice(), _vk.GetDevice(), _size, _bufferUsageFlags, _bufferMemoryPropertyFlags, &_buffer, &_bufferMemory);

		if (!nullMemory) GvkHelper::write_to_buffer(_vk.GetDevice(), _bufferMemory, data, _size);
	}

	Buffer(void* data, VkBufferUsageFlags usage)
	{
		_size = sizeof(data);
		_bufferUsageFlags = usage;
		_bufferMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		GvkHelper::create_buffer(_vk.GetPhysicalDevice(), _vk.GetDevice(), _size, _bufferUsageFlags, _bufferMemoryPropertyFlags, &_buffer, &_bufferMemory);
		GvkHelper::write_to_buffer(_vk.GetDevice(), _bufferMemory, data, _size);
	}

	~Buffer()
	{
		vkDestroyBuffer(_vk.GetDevice(), _buffer, nullptr);
		vkFreeMemory(_vk.GetDevice(), _bufferMemory, nullptr);
	}

	VkBuffer& GetBuffer() { return _buffer; }
	VkDeviceMemory& GetMemory() { return _bufferMemory; }
	VkDeviceSize& GetSize() { return _size; }

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
	VkClearValue _clearValue = { .color {0, 0, 0, 1} };

	VkImageLayout _imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

public:
	Texture()
	{

	}

	Texture(VkImageAspectFlags imageAspectFlags, VkFormat format)
	{
		_extent = { _vk.GetWidth(), _vk.GetHeight(), 1 };
		_sampleCountFlagBits = VK_SAMPLE_COUNT_1_BIT;
		_format = format;
		_imageTiling = VK_IMAGE_TILING_OPTIMAL;
		_imageAspectFlags = imageAspectFlags;
		//_imageUsageFlags = (_imageAspectFlags & VK_IMAGE_ASPECT_DEPTH_BIT) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		//_imageMemoryPropertyFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		_imageUsageFlags = (_imageAspectFlags & VK_IMAGE_ASPECT_DEPTH_BIT) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		_imageMemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		GvkHelper::create_image(_vk.GetPhysicalDevice(), _vk.GetDevice(), _extent, 1, _sampleCountFlagBits, _format, _imageTiling, _imageUsageFlags, _imageMemoryPropertyFlags, nullptr, &_image, &_imageMemory);
		GvkHelper::create_image_view(_vk.GetDevice(), _image, _format, _imageAspectFlags, 1, nullptr, &_imageView);
	}

	Texture(const std::string imgPath)
	{

	}

	Texture(const tinygltf::Image glImage)
	{
		_extent = { (unsigned)glImage.width, (unsigned)glImage.height, 1 };
		VkDeviceSize size = glImage.width * glImage.height * glImage.component;

		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		if (glImage.bits == 16) format = VK_FORMAT_R16G16B16A16_SFLOAT;
		else if (glImage.bits == 32) format = VK_FORMAT_R32G32B32A32_SFLOAT;

		Buffer staging(size, 0, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
		staging.WriteToBuffer(glImage.image.data());

		unsigned int mipLevels = static_cast<unsigned int>(floor(log2(std::max(glImage.width, glImage.height))) + 1);
		GvkHelper::create_image(_vk.GetPhysicalDevice(), _vk.GetDevice(), _extent, mipLevels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &_image, &_imageMemory);
		
		VkCommandBuffer copyCommandBuffer;
		GvkHelper::signal_command_start(_vk.GetDevice(), _vk.GetCommandPool(), &copyCommandBuffer);

		VkBufferImageCopy bufferImageCopy
		{
			.bufferOffset = 0,
			.imageSubresource
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = mipLevels,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.imageOffset = 0,
			.imageExtent = _extent,
		};

		_vk.TransitionImageLayout(copyCommandBuffer, 1, 1, _image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		//copy from staging
		vkCmdCopyBufferToImage(copyCommandBuffer, staging.GetBuffer(), _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

		_vk.TransitionImageLayout(copyCommandBuffer, 1, 1, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		GvkHelper::signal_command_end(_vk.GetDevice(), _vk.GetGraphicsQueue(), _vk.GetCommandPool(), &copyCommandBuffer);

		GvkHelper::create_image_view(_vk.GetDevice(), _image, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, nullptr, &_imageView);



	}

	~Texture()
	{
		vkDestroyImage(_vk.GetDevice(), _image, nullptr);
		vkDestroyImageView(_vk.GetDevice(), _imageView, nullptr);
		vkFreeMemory(_vk.GetDevice(), _imageMemory, nullptr);
	}

	VkImage& GetImage() { return _image; }
	VkImageView& GetImageView() { return _imageView; }
	//VkImageView GetImageViewX() { return _imageView; }
	VkExtent3D& GetExtent() { return _extent; }
	VkClearValue& GetClearValue() { return _clearValue; }
	VkFormat GetFormat() const { return _format; }
	void SetFormat(VkFormat format) { _format = format; }

	void SetImageLayout(VkImageLayout newLayout);
	void SetImageLayout(VkCommandBuffer& commandBuffer, VkImageLayout newLayout);
	void TransitionLayout(VkCommandBuffer& commandBuffer);
	void TransitionLayout();

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