#include "pch.hpp"
#include "Vulkan.hpp"
#include "gltf/stb_image.h"
#include "EngineShaders.h"
#include "Imgn/UI/UIShader.h"

void Vulkan::CreateImageView(vk::Format pFormat, vk::ImageAspectFlags pAspectFlags, Image& pImage)
{
	vk::ImageViewCreateInfo imageViewCreateInfo
	{
		.image = *pImage.image,
		.viewType = vk::ImageViewType::e2D,
		.format = pFormat,
		.components
		{
			.r = vk::ComponentSwizzle::eIdentity,
			.g = vk::ComponentSwizzle::eIdentity,
			.b = vk::ComponentSwizzle::eIdentity,
			.a = vk::ComponentSwizzle::eIdentity
		},
		.subresourceRange
		{
			.aspectMask = pAspectFlags,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	pImage.view = Unique<vk::raii::ImageView>(_device->createImageView(imageViewCreateInfo));
}

void Vulkan::CreateBuffer(vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Buffer& pBuffer)
{
	if (pSize == 0)
	{
		throw std::invalid_argument(
			"Vulkan::CreateBuffer called with pSize == 0"
		);
	}

	vk::BufferCreateInfo bufferCreateInfo
	{
		.size = pSize,
		.usage = pUsage,
		.sharingMode = vk::SharingMode::eExclusive
	};

	pBuffer.buffer = Unique<vk::raii::Buffer>(_device->createBuffer(bufferCreateInfo));

	vk::MemoryRequirements memReqs = pBuffer.buffer->getMemoryRequirements();
	vk::MemoryAllocateInfo memoryAllocateInfo
	{
		.allocationSize = memReqs.size,
		.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, pProps)
	};

	pBuffer.memory = Unique<vk::raii::DeviceMemory>(_device->allocateMemory(memoryAllocateInfo));

	pBuffer.buffer->bindMemory(*pBuffer.memory, 0);
}

void Vulkan::CreateImage(uint32_t pWidth, uint32_t pHeight, vk::Format pFormat, vk::ImageTiling pTiling, vk::ImageUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Image& pImage)
{
	vk::ImageCreateInfo imageCreateInfo
	{
		.imageType = vk::ImageType::e2D,
		.format = pFormat,
		.extent = {pWidth , pHeight, 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = vk::SampleCountFlagBits::e1,
		.tiling = pTiling,
		.usage = pUsage,
		.sharingMode = vk::SharingMode::eExclusive
	};

	pImage.image = Unique<vk::raii::Image>(_device->createImage(imageCreateInfo));

	vk::MemoryRequirements memReqs = pImage.image->getMemoryRequirements();
	vk::MemoryAllocateInfo memoryAllocateInfo
	{
		.allocationSize = memReqs.size,
		.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, pProps)
	};

	pImage.memory = Unique<vk::raii::DeviceMemory>(_device->allocateMemory(memoryAllocateInfo));
	pImage.image->bindMemory(*pImage.memory, 0);
}

void Vulkan::Init(RendererCreateInfo pCreateInfo)
{
	_info = pCreateInfo;

	CreateDXC();
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface(_info.windowHandle);
	PickPhysicalDevice();
	CreateDevice();
	CreateSwapchain();
	CreateSwapchainImageViews();
	CreateDescriptorSetLayout();
	CreateGraphicsPipelines();
	CreateCommandPool();
	//CreateDepthResources();
	//CreateTextureImage();
	//CreateTextureImageView();
	CreateTextureSampler();
	CreateTAASampler();
	//CreateVertexBuffer();
	//CreateIndexBuffer();
	//CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffers();
	CreateSyncObjects();
}

bool Vulkan::StartFrame()
{
	_device->waitForFences(**_frameFinishedFence[_frameInFlightIdx], vk::True, UINT64_MAX);
	//_device->resetFences(**_frameFinishedFence[_frameInFlightIdx]);

	try
	{
		auto result = _swapchain->acquireNextImage(UINT64_MAX, *_imageAcquiredSemaphores[_frameInFlightIdx]);

		_activeImageIdx = *result;

		if (result.result == vk::Result::eSuboptimalKHR)
		{
			RecreateSurfaceAndSwapchain();
			return false;
		}
	}
	catch (const vk::OutOfDateKHRError&)
	{
		RecreateSurfaceAndSwapchain();
		return false;
	}
	catch (const vk::SurfaceLostKHRError&)
	{
		RecreateSurfaceAndSwapchain();
		return false;
	}	//_activeImageIdx = imageIdx;


	vk::CommandBufferBeginInfo beginInfo
	{
		.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
	};

	vk::raii::CommandBuffer& commandBuffer = *_commandBuffers[_frameInFlightIdx];

	commandBuffer.reset();
	commandBuffer.begin(beginInfo);

	return true;
}

void Vulkan::BlitToSwapchain(RGImage& pImage)
{
	if (!pImage.image.image)
		throw std::runtime_error("EndFrame received a null render image");

	vk::raii::CommandBuffer& commandBuffer = *_commandBuffers[_frameInFlightIdx];

	TransitionImageLayout(*_commandBuffers[_frameInFlightIdx], vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, _swapchainImages[_activeImageIdx]);
	TransitionImageLayout(*_commandBuffers[_frameInFlightIdx], pImage.currentLayout, vk::ImageLayout::eTransferSrcOptimal, *pImage.image.image, pImage.aspect);

	pImage.currentLayout = vk::ImageLayout::eTransferSrcOptimal;
	//blit final color for now
	vk::ImageSubresourceLayers subresource
	{
		.aspectMask = vk::ImageAspectFlagBits::eColor,
		.mipLevel = 0,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	vk::ImageBlit blit
	{
		.srcSubresource = subresource,
		.dstSubresource = subresource,
	};

	vk::Offset3D blitOffset0{ 0, 0, 0 };
	//vk::Offset3D blitOffsetSrc1{ (int32_t)renderWidth, (int32_t)renderHeight, 1 };
	vk::Offset3D blitOffsetDst1{ (int32_t)_swapchainExtent.width, (int32_t)_swapchainExtent.height, 1 };
	blit.srcOffsets[0] = blitOffset0;
	blit.srcOffsets[1] = vk::Offset3D{ static_cast<int32_t>(pImage.width), static_cast<int32_t>(pImage.height), 1 };;
	blit.dstOffsets[0] = blitOffset0;
	blit.dstOffsets[1] = blitOffsetDst1;

	//auto finalColor = _graph.GetResource("FinalColor");
	_commandBuffers[_frameInFlightIdx]->blitImage(*pImage.image.image, vk::ImageLayout::eTransferSrcOptimal, _swapchainImages[_activeImageIdx], vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

	TransitionImageLayout(*_commandBuffers[_frameInFlightIdx], vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, *pImage.image.image, pImage.aspect);

	pImage.currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	TransitionImageLayout(*_commandBuffers[_frameInFlightIdx], vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eColorAttachmentOptimal, _swapchainImages[_activeImageIdx]);
}

void Vulkan::EndFrame()
{
	vk::raii::CommandBuffer& commandBuffer = *_commandBuffers[_frameInFlightIdx];
	TransitionImageLayout(*commandBuffer, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, _swapchainImages[_activeImageIdx]);
	commandBuffer.end();

	_device->resetFences(**_frameFinishedFence[_frameInFlightIdx]);

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eTransfer);

	const vk::SubmitInfo submitInfo
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &**_imageAcquiredSemaphores[_frameInFlightIdx],
		.pWaitDstStageMask = &waitDestinationStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &**_commandBuffers[_frameInFlightIdx],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &**_presentationReadySemaphore[_activeImageIdx]
	};

	_queue->submit(submitInfo, *_frameFinishedFence[_frameInFlightIdx]);

	const vk::PresentInfoKHR presentInfoKHR
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &**_presentationReadySemaphore[_activeImageIdx],
		.swapchainCount = 1,
		.pSwapchains = &**_swapchain,
		.pImageIndices = &_activeImageIdx
	};

	try
	{
		vk::Result result = _queue->presentKHR(presentInfoKHR);

		if (result == vk::Result::eSuboptimalKHR)
		{
			RecreateSwapchain();
			return;
		}
	}
	catch (const vk::OutOfDateKHRError&)
	{
		RecreateSurfaceAndSwapchain();
		return;
	}
	catch (const vk::SurfaceLostKHRError&)
	{
		RecreateSurfaceAndSwapchain();
		return;
	}

	_frameInFlightIdx = (_frameInFlightIdx + 1) % MaxFramesInFlight;
}

Buffer Vulkan::CreateVertexBuffer(void* pData, uint64_t pSize)
{
	Buffer vertexBuffer;

	Buffer staging;
	CreateBuffer(pSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);

	void* stagingData = staging.memory->mapMemory(0, pSize);
	memcpy(stagingData, pData, pSize);
	staging.memory->unmapMemory();

	CreateBuffer(pSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer);

	CopyBuffer(*staging.buffer, *vertexBuffer.buffer, pSize);

	return vertexBuffer;
}

Buffer Vulkan::CreateIndexBuffer(void* pData, uint64_t pSize)
{
	Buffer indexBuffer;

	Buffer staging;
	CreateBuffer(pSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);

	void* stagingData = staging.memory->mapMemory(0, pSize);
	memcpy(stagingData, pData, pSize);
	staging.memory->unmapMemory();

	CreateBuffer(pSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer);

	CopyBuffer(*staging.buffer, *indexBuffer.buffer, pSize);

	return indexBuffer;
}

Buffer Vulkan::CreateUniformBuffer(void* pData, uint64_t pSize)
{
	Buffer uniformBuffer;

	CreateBuffer(pSize, vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uniformBuffer);

	if (!pData) return uniformBuffer;

	Buffer stagingBuffer;

	CreateBuffer(pSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer);

	void* mappedData = stagingBuffer.memory->mapMemory(0, pSize);
	std::memcpy(mappedData, pData, pSize);
	stagingBuffer.memory->unmapMemory();

	CopyBuffer(*stagingBuffer.buffer, *uniformBuffer.buffer, pSize);

	return uniformBuffer;
}

Buffer Vulkan::CreateStorageBuffer(void* pData, uint64_t pSize)
{
	Buffer storageBuffer;

	CreateBuffer(pSize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, storageBuffer);

	if (!pData) return storageBuffer;

	Buffer stagingBuffer;

	CreateBuffer(pSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer);

	void* mappedData = stagingBuffer.memory->mapMemory(0, pSize);
	std::memcpy(mappedData, pData, pSize);
	stagingBuffer.memory->unmapMemory();

	CopyBuffer(*stagingBuffer.buffer, *storageBuffer.buffer, pSize);

	return storageBuffer;
}


Buffer Vulkan::CreateMappedStorageBuffer(uint64_t pSize)
{
	Buffer storageBuffer;
	CreateBuffer(pSize, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, storageBuffer);
	return storageBuffer;
}

RGBuffer Vulkan::CreateRenderBuffer(void* pData, uint64_t pSize, vk::BufferUsageFlags pUsage)
{
	RGBuffer buffer;

	if (pUsage & vk::BufferUsageFlagBits::eUniformBuffer)
	{
		buffer.buffer = CreateUniformBuffer(pData, pSize);
	}
	else if (pUsage & vk::BufferUsageFlagBits::eStorageBuffer)
	{
		buffer.buffer = CreateStorageBuffer(pData, pSize);
	}

	return buffer;
}

void Vulkan::MapBufferData(void* pData, uint64_t pSize, Buffer* pBuffer)
{
	if (!pData) IMGN_WARN("Data trying to be mapped is null");

	std::memcpy(pBuffer->memory->mapMemory(0, pSize), pData, pSize);
	pBuffer->memory->unmapMemory();
}

vk::raii::Semaphore Vulkan::CreateVkSemaphore()
{
	return _device->createSemaphore(vk::SemaphoreCreateInfo());
}

Image Vulkan::CreateDepthImage(uint32_t pWidth, uint32_t pHeight)
{
	Image depth;

	CreateImage(pWidth, pHeight, vk::Format::eD32Sfloat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depth);
	CreateImageView(vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth, depth);

	return depth;
}

Image Vulkan::CreateTextureImage(uint32_t pWidth, uint32_t pHeight, const uint8_t* pData)
{
	vk::DeviceSize imageSize = pWidth * pHeight * 4;

	Buffer staging;
	CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);

	void* data = staging.memory->mapMemory(0, imageSize);
	memcpy(data, pData, imageSize);
	staging.memory->unmapMemory();

	Image texture;
	CreateImage(pWidth, pHeight, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, texture);
	CreateImageView(vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, texture);

	TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, *texture.image);
	CopyBufferToImage(pWidth, pHeight, *staging.buffer, *texture.image);
	TransitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, *texture.image);

	return texture;
}

Image Vulkan::CreateTextureImage(const std::string& pFile)
{
	int width, height, channels;

	stbi_set_flip_vertically_on_load(true);
	uint8_t* pixels = stbi_load(pFile.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	vk::DeviceSize imageSize = width * height * 4;

	Buffer staging;
	CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);

	void* data = staging.memory->mapMemory(0, imageSize);
	memcpy(data, pixels, imageSize);
	staging.memory->unmapMemory();

	stbi_image_free(pixels);

	Image texture;
	CreateImage(static_cast<int>(width), static_cast<int>(height), vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, texture);
	CreateImageView(vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, texture);

	TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, *texture.image);
	CopyBufferToImage(static_cast<int>(width), static_cast<int>(height), *staging.buffer, *texture.image);
	TransitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, *texture.image);

	return texture;
}

RGImage Vulkan::CreateRenderImage(uint32_t pWidth, uint32_t pHeight, vk::Format pFormat, vk::ImageAspectFlags pAspect)
{
	RGImage image;
	image.width = pWidth; image.height = pHeight;
	image.format = pFormat;
	image.aspect = pAspect;

	if (image.aspect & vk::ImageAspectFlagBits::eDepth)
	{
		CreateImage(image.width, image.height, image.format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, image.image);
		CreateImageView(image.format, image.aspect, image.image);

		return image;
	}

	CreateImage(image.width, image.height, image.format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, image.image);
	CreateImageView(image.format, image.aspect, image.image);

	return image;
}

void Vulkan::UpdateImageDescriptor(uint32_t pSlot, const Image& pImage)
{
	if (pSlot >= NumDescriptorsStreaming)
	{
		IMGN_FATAL("Image slot {} is greater than NumDescriptorStreaming {}", pSlot, NumDescriptorsStreaming);
		throw std::runtime_error("Image descriptor slot exceeds NumDescriptorsStreaming");
	}
	if (!pImage.view)
	{
		IMGN_FATAL("Image view is null");
		throw std::runtime_error("Cannot update descriptor with a null image view");
	}

	vk::DescriptorImageInfo imageInfo
	{
		.sampler = **_textureSampler,
		.imageView = **pImage.view,
		.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
	};

	vk::WriteDescriptorSet write
	{
		.dstSet = **_textureDescriptorSet,
		.dstBinding = 0,
		.dstArrayElement = pSlot,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eCombinedImageSampler,
		.pImageInfo = &imageInfo
	};

	std::array writes = { write };

	_device->updateDescriptorSets(writes, {});
}
