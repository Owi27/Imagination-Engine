#pragma once
#include "pch.h"
#include "VkResources.h"

struct QueueFamilyIndices
{
	std::optional<unsigned> graphicsFamily = 0;
	std::optional<unsigned> presentFamily = 0;

	bool IsComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct VkCtx
{
	static VkCtx& Instance()
	{
		if (!_instance) _instance.reset(new VkCtx());

		return *_instance;
	}

	VkInstance instance = nullptr;
	VkSurfaceKHR surface = nullptr;
	QueueFamilyIndices queueFamilyIndices;
	VkPhysicalDevice physicalDevice = nullptr;
	VkDevice device = nullptr;
	VkQueue graphicsQueue = nullptr;
	VkQueue presentQueue = nullptr;
	VkPipelineCache pipelineCache = nullptr;
	VkDescriptorPool descriptorPool = nullptr;
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
	VkCommandPool commandPool = nullptr;
	unsigned currentFrame = 0, targetFrame = 0, maxFrame = 0;
	std::vector<VkSemaphore> imageAvailableSemaphores, renderFinishedSemaphores;
	std::vector<VkFence> renderingFences;
	uint64_t uniformSize, storageSize, combinedSamplerSize, setAlign;
	VkExtent3D swapchainExtent;
	VkSampler sampler;

	PFN_vkGetDescriptorEXT vkGetDescriptorEXT;
	PFN_vkGetDescriptorSetLayoutSizeEXT vkGetDescriptorSetLayoutSizeEXT;
	PFN_vkGetDescriptorSetLayoutBindingOffsetEXT vkGetDescriptorSetLayoutBindingOffsetEXT;
	PFN_vkCmdBindDescriptorBuffersEXT vkCmdBindDescriptorBuffersEXT;
	PFN_vkCmdSetDescriptorBufferOffsetsEXT vkCmdSetDescriptorBufferOffsetsEXT;

	std::shared_ptr<Texture> depth;

	VkCtx(VkCtx& window) = delete;
	void operator=(const VkCtx&) = delete;
	~VkCtx() = default;

private:
	static inline std::unique_ptr<VkCtx> _instance;

	VkCtx() = default;
};

namespace VkHelpers
{

	static VkCommandBuffer StartCommandBuffer()
	{
		VkCommandBuffer commandBuffer;

		VkCommandBufferAllocateInfo commandBufferAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			//.pNext = ,
			.commandPool = VkCtx::Instance().commandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		vkAllocateCommandBuffers(VkCtx::Instance().device, &commandBufferAllocateInfo, &commandBuffer);

		VkCommandBufferBeginInfo commandBufferBeginInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			//.pNext = ,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			//.pInheritanceInfo = ,
		};

		vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

		return commandBuffer;
	}

	static void EndCommandBuffer(VkCommandBuffer pCommandBuffer)
	{
		vkEndCommandBuffer(pCommandBuffer);

		VkSubmitInfo submitInfo
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			//.pNext = ,
			//.waitSemaphoreCount = ,
			//.pWaitSemaphores = ,
			//.pWaitDstStageMask = ,
			.commandBufferCount = 1,
			.pCommandBuffers = &pCommandBuffer,
			//.signalSemaphoreCount = ,
			//.pSignalSemaphores = ,
		};

		vkQueueSubmit(VkCtx::Instance().graphicsQueue, 1, &submitInfo, nullptr);
		vkQueueWaitIdle(VkCtx::Instance().graphicsQueue);
		vkFreeCommandBuffers(VkCtx::Instance().device, VkCtx::Instance().commandPool, 1, &pCommandBuffer);
	}

	inline uint64_t AlignUp(uint64_t pV, uint64_t pAlignment)
	{
		return (pV + pAlignment - 1) & ~(pAlignment - 1);
	};
}

struct Descriptor
{
	VkDescriptorSetLayout layout;
	bool hasImage = false, hasResource = false;
	std::unique_ptr<Buffer> descriptorBuffer, resourceDescriptorBuffer, imageDescriptorBuffer;
	uint64_t layoutSize, deviceAddress, resourceDescriptorSize, resourceDescriptorOffset, imageDescriptorSize, imageDescriptorOffset;
	std::vector<uint64_t> bindingOffsets;
	std::vector<VkDescriptorAddressInfoEXT> addressInfo;
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings; //0: scenedata, 1: material, 2: textures

	Descriptor& AddLayoutBinding(unsigned pBinding, DescriptorType pType, ShaderStage pStageFlags, unsigned pCount = 1)
	{
		if (pType == DescriptorType::UNIFORM_BUFFER || pType == DescriptorType::STORAGE_BUFFER) hasResource = true;
		if (pType == DescriptorType::IMAGE_SAMPLER || pType == DescriptorType::INPUT_ATTACHMENT) hasImage = true;

		VkDescriptorSetLayoutBinding layoutBinding
		{
			.binding = pBinding,
			.descriptorType = (VkDescriptorType)pType,
			.descriptorCount = pCount,
			.stageFlags = (unsigned)pStageFlags,
			//.pImmutableSamplers = ,
		};

		layoutBindings.push_back(layoutBinding);

		return *this;
	}

	Descriptor& PrepareDescriptorBuffers()
	{
		const uint64_t totalSize = layoutSize * VkCtx::Instance().maxFrame;
		descriptorBuffer = std::make_unique<Buffer>();
		descriptorBuffer->CreateMappedBuffer(totalSize, BufferUsage::DESCRIPTOR | BufferUsage::DESCRIPTOR_SAMPLER | BufferUsage::DEVICE_ADDRESS, MemoryFlags::CPU2GPU);

		return *this;
	}

	Descriptor& CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutCreateInfo layoutCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			//.pNext = ,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
			.bindingCount = static_cast<uint32_t>(layoutBindings.size()),
			.pBindings = layoutBindings.data(),
		};

		vkCreateDescriptorSetLayout(VkCtx::Instance().device, &layoutCreateInfo, nullptr, &layout);

		VkCtx::Instance().vkGetDescriptorSetLayoutSizeEXT(VkCtx::Instance().device, layout, &layoutSize);
		layoutSize = VkHelpers::AlignUp(layoutSize, VkCtx::Instance().setAlign);

		bindingOffsets.resize(layoutBindings.size());
		for (size_t i = 0; i < bindingOffsets.size(); i++)
		{
			VkCtx::Instance().vkGetDescriptorSetLayoutBindingOffsetEXT(VkCtx::Instance().device, layout, layoutBindings[i].binding, &bindingOffsets[i]);
		}

		return *this;
	}

	Descriptor& AddAddressInfo(int pBindingIdx, int pSetIdx, uint64_t pAddress, uint64_t pRange, DescriptorType pType)
	{
		VkDescriptorAddressInfoEXT descriptorAddressInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT,
			//.pNext = ,
			.address = pAddress,
			.range = pRange,
			.format = VK_FORMAT_UNDEFINED,
		};

		VkDescriptorGetInfoEXT descriptorGetInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
			//.pNext = ,
			.type = (VkDescriptorType)pType,
		};

		uint64_t descriptorSize = 0;

		switch (pType)
		{
		case DescriptorType::UNIFORM_BUFFER:
			descriptorGetInfo.data.pUniformBuffer = &descriptorAddressInfo;
			descriptorSize = VkCtx::Instance().uniformSize;
			break;

		case DescriptorType::STORAGE_BUFFER:
			descriptorGetInfo.data.pStorageBuffer = &descriptorAddressInfo;
			descriptorSize = VkCtx::Instance().storageSize;
			break;

		default:
			break;
		}

		char* descriptorBufferPointer = static_cast<char*>(descriptorBuffer->GetData());
		uint64_t baseOffset = (uint64_t)layoutSize * (uint64_t)pSetIdx;     // stride * set
		uint64_t bindingOffset = (uint64_t)bindingOffsets[pBindingIdx];
		uint64_t finalOffset = baseOffset + bindingOffset;

		char* destination = descriptorBufferPointer + finalOffset;

		VkCtx::Instance().vkGetDescriptorEXT(VkCtx::Instance().device, &descriptorGetInfo, descriptorSize, destination);

		return *this;
	}

	Descriptor& AddAddressInfo(int pBindingIdx, int pSetIdx, int pArrayIdx, VkImageView pImageView, DescriptorType pType)
	{
		VkDescriptorImageInfo descriptorImageInfo
		{
			.sampler = VkCtx::Instance().sampler,
			.imageView = pImageView,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		VkDescriptorGetInfoEXT descriptorGetInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
			//.pNext = ,
			.type = (VkDescriptorType)pType,
		};

		uint64_t descriptorSize = 0;

		switch (pType)
		{
		case DescriptorType::IMAGE_SAMPLER:
			descriptorGetInfo.data.pCombinedImageSampler = &descriptorImageInfo;
			descriptorSize = VkCtx::Instance().combinedSamplerSize;
			break;

		default:
			break;
		}

		char* descriptorBufferPointer = static_cast<char*>(descriptorBuffer->GetData());
		uint64_t baseOffset = (uint64_t)layoutSize * (uint64_t)pSetIdx;     // stride * set
		uint64_t bindingOffset = (uint64_t)bindingOffsets[pBindingIdx];
		uint64_t finalOffset = baseOffset + bindingOffset + (uint64_t)pArrayIdx * (uint64_t)descriptorSize;

		char* destination = descriptorBufferPointer + finalOffset;

		VkCtx::Instance().vkGetDescriptorEXT(VkCtx::Instance().device, &descriptorGetInfo, descriptorSize, destination);

		return *this;
	}
};