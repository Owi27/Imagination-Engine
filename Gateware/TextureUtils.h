#ifndef TEXTUREUTILS_H
#define TEXTUREUTILS_H

// Requires tinygltf.h and Gateware.h

// function to upload a texture to the GPU
inline void UploadTextureToGPU(GW::GRAPHICS::GVulkanSurface _surface, const tinygltf::Image& _img, 
						VkBuffer& _outTextureBuffer, VkDeviceMemory& _outTextureMemory, 
						VkImage& _outTextureImage, VkImageView& _outTextureImageView)
{
	// grab all the needed handles
	VkQueue vkQGX;
	VkDevice vkDev;
	VkPhysicalDevice vkPDev;
	VkCommandPool vkCmdPool;
	_surface.GetDevice(reinterpret_cast<void**>(&vkDev));
	_surface.GetGraphicsQueue(reinterpret_cast<void**>(&vkQGX));
	_surface.GetPhysicalDevice(reinterpret_cast<void**>(&vkPDev));
	_surface.GetCommandPool(reinterpret_cast<void**>(&vkCmdPool));
	
	//Set up Texture staging buffer
	VkDeviceSize imageSize = _img.width * _img.height * _img.component;
	VkBuffer staging_bufferIM; // temp, will be cleaned up
	VkDeviceMemory staging_buffer_memoryIM; // temp, will be cleaned up
	VkDeviceMemory transitionMemory; // temp, will be cleaned up

	// determine format
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM; // 8bit default
	if (_img.bits == 16) 
		format = VK_FORMAT_R16G16B16A16_SFLOAT;
	else if (_img.bits == 32) 
		format = VK_FORMAT_R32G32B32A32_SFLOAT;

	//Create the staging buffers
	GvkHelper::create_buffer(vkPDev, vkDev, imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&staging_bufferIM, &staging_buffer_memoryIM);

	//Copy the staging information over
	GvkHelper::write_to_buffer(vkDev, staging_buffer_memoryIM,
		_img.image.data(), static_cast<unsigned int>(imageSize));
	//Create the new Buffer
	GvkHelper::create_buffer(vkPDev, vkDev, imageSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&_outTextureBuffer, &transitionMemory);

	//Copy the staging buffer data to the new buffer
	GvkHelper::copy_buffer(vkDev, vkCmdPool, vkQGX, staging_bufferIM, _outTextureBuffer, imageSize);

	VkExtent3D tempExtent = { _img.width, _img.height, 1 };
	uint32_t mipLevels = static_cast<uint32_t>( floor( log2( std::max(_img.width, _img.height))) + 1);
	GvkHelper::create_image(vkPDev, vkDev, tempExtent, mipLevels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, 
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &_outTextureImage, &_outTextureMemory);

	//transition
	GvkHelper::transition_image_layout(vkDev, vkCmdPool, vkQGX, mipLevels, _outTextureImage, format,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	GvkHelper::copy_buffer_to_image(vkDev, vkCmdPool, vkQGX, staging_bufferIM, _outTextureImage, tempExtent);

	//create mipmaps
	GvkHelper::create_mipmaps(vkDev, vkCmdPool, vkQGX, _outTextureImage, _img.width, _img.height, mipLevels);

	GvkHelper::create_image_view(vkDev, _outTextureImage, format,
		VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, nullptr, &_outTextureImageView);

	vkDestroyBuffer(vkDev, staging_bufferIM, nullptr);
	vkFreeMemory(vkDev, staging_buffer_memoryIM, nullptr); //staging buffer IM cleaned
	vkFreeMemory(vkDev, transitionMemory, nullptr); //staging buffer IM cleaned
}

inline VkResult CreateSampler(	GW::GRAPHICS::GVulkanSurface _surface, VkSampler& _outSampler,
						VkSamplerAddressMode _addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
						VkFilter _filter = VK_FILTER_LINEAR, float _anisotropy = 4.0f)
{
	// grab all the needed handles
	VkDevice vkDev;
	_surface.GetDevice(reinterpret_cast<void**>(&vkDev));

	// create the the image view and sampler
	VkSamplerCreateInfo samplerInfo = {};
	// Set the struct values
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.flags = 0;
	samplerInfo.addressModeU = _addressMode; // REPEAT IS COMMON
	samplerInfo.addressModeV = _addressMode;
	samplerInfo.addressModeW = _addressMode;
	samplerInfo.magFilter = _filter;
	samplerInfo.minFilter = _filter;
	samplerInfo.mipmapMode = (_filter == VK_FILTER_NEAREST) 
		? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0;
	samplerInfo.minLod = 0;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
	samplerInfo.anisotropyEnable = (_anisotropy >= 1.0f) ? VK_TRUE : VK_FALSE;
	samplerInfo.maxAnisotropy = _anisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_LESS;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.pNext = nullptr;

	return vkCreateSampler(vkDev, &samplerInfo, nullptr, &_outSampler);
}

#endif // !TEXTUREUTILS_H