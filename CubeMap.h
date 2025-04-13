#pragma once
class CubeMap
{
	VulkanContext& _vk;
	unsigned char* _textureData[6];

	VkImage _image;
	VkDeviceMemory _imageMemory;
	VkImageView _imageView;

	std::vector<std::unique_ptr<Texture>> _textures;

public:
	CubeMap(std::vector<std::string> imgPaths) : _vk(*VulkanContext::GetInst())
	{
		int width, height, component;
		for (size_t i = 0; i < imgPaths.size(); i++)
		{
			_textureData[i] = stbi_load(imgPaths[i].c_str(), &width, &height, &component, STBI_rgb_alpha);
			component = 4;
		}

		const VkDeviceSize imageSize = width * height * 4 * 6;
		const VkDeviceSize layerSize = imageSize / 6;

		Buffer staging(imageSize, 0, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

		void* data;
		vkMapMemory(_vk.GetDevice(), staging.GetMemory(), 0, imageSize, 0, &data);

		for (size_t i = 0; i < 6; i++)
		{
			memcpy(static_cast<char*>(data) + layerSize * i, _textureData[i], static_cast<size_t>(layerSize));
		}

		vkUnmapMemory(_vk.GetDevice(), staging.GetMemory());

		VkImageCreateInfo imageCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_UNORM,
			.extent = {width, height, 1},
			.mipLevels = 1,
			.arrayLayers = 6,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};

		vkCreateImage(_vk.GetDevice(), &imageCreateInfo, nullptr, &_image);

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(_vk.GetDevice(), _image, &memoryRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memoryRequirements.size,
			.memoryTypeIndex = _vk.FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		vkAllocateMemory(_vk.GetDevice(), &memoryAllocateInfo, nullptr, &_imageMemory);
		vkBindImageMemory(_vk.GetDevice(), _image, _imageMemory, 0);
		
		VkCommandBuffer copyCommandBuffer;
		GvkHelper::signal_command_start(_vk.GetDevice(), _vk.GetCommandPool(), &copyCommandBuffer);

		std::vector<VkBufferImageCopy> bufferImageCopies;
		unsigned offset = 0;

		for (unsigned face = 0; face < 6; face++)
		{
			for (unsigned level = 0; level < 1; level++) //todo un hardcode mip level
			{
				VkBufferImageCopy bufferImageCopy
				{
					.bufferOffset = face * layerSize,
					.imageSubresource
					{
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.mipLevel = level,
						.baseArrayLayer = face,
						.layerCount = 1
					},
					.imageExtent = {width, height, 1}
				};

				bufferImageCopies.push_back(bufferImageCopy);
			}
		}

		_vk.TransitionImageLayout(1, 6, _image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		//copy from staging
		vkCmdCopyBufferToImage(copyCommandBuffer, staging.GetBuffer(), _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (unsigned)bufferImageCopies.size(), bufferImageCopies.data());

		_vk.TransitionImageLayout(1, 6, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		GvkHelper::signal_command_end(_vk.GetDevice(), _vk.GetGraphicsQueue(), _vk.GetCommandPool(), &copyCommandBuffer);

		VkImageViewCreateInfo imageViewCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			//.flags =
			.image = _image,
			.viewType = VK_IMAGE_VIEW_TYPE_CUBE,
			.format = VK_FORMAT_R8G8B8A8_UNORM,
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 6
			}
		};

		vkCreateImageView(_vk.GetDevice(), &imageViewCreateInfo, nullptr, &_imageView);
	}

	~CubeMap()
	{
		for (size_t i = 0; i < 6; i++)
		{
			stbi_image_free(_textureData[i]);
		}

		vkFreeMemory(_vk.GetDevice(), _imageMemory, nullptr);
		vkDestroyImage(_vk.GetDevice(), _image, nullptr);
		vkDestroyImageView(_vk.GetDevice(), _imageView, nullptr);
	}
};

