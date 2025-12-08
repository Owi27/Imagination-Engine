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

	std::shared_ptr<Texture> depth;

	VkCtx(VkCtx& window) = delete;
	void operator=(const VkCtx&) = delete;
	~VkCtx() = default;

private:
	static inline std::unique_ptr<VkCtx> _instance;

	VkCtx() = default;
};

struct Descriptor
{
	VkDescriptorSetLayout layout;
	Buffer descriptorBuffer;
	uint64_t layoutSize;
	std::vector<uint64_t> bindingOffsets;
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings; //0: scenedata, 1: material, 2: textures

	Descriptor& AddLayoutBinding(unsigned pBinding, DescriptorType pType, ShaderStage pStageFlags, unsigned pCount = 1)
	{
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

	void CreateDescriptorSetLayout()
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

		bindingOffsets.resize(layoutBindings.size());
		for (size_t i = 0; i < bindingOffsets.size(); i++)
		{
			vkGetDescriptorSetLayoutBindingOffsetEXT(VkCtx::Instance().device, layout, i, &bindingOffsets[i]);
		}

		uint64_t rawLayoutSize;
		vkGetDescriptorSetLayoutSizeEXT(VkCtx::Instance().device, layout, &rawLayoutSize);

		VkHelpers::AlignUp(rawLayoutSize, VkCtx::Instance().setAlign);

		descriptorBuffer.CreateBuffer(layoutSize, BufferUsage::DESCRIPTOR | BufferUsage::DESCRIPTOR_SAMPLER | BufferUsage::DEVICE_ADDRESS, MemoryFlags::CPU | MemoryFlags::CPU2GPU);
	}
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

	uint64_t AlignUp(uint64_t pV, uint64_t pAlignment)
	{
		return (pV + pAlignment - 1) & ~(pAlignment - 1);
	};
}