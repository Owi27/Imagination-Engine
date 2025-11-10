#pragma once
#include <optional>
#include <IWindow.h>
#undef LoadImage

struct VulkanContext
{
	struct QueueFamilyIndices
	{
		std::optional<unsigned> graphicsFamily = 0;
		std::optional<unsigned> presentFamily = 0;

		bool IsComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
	} queueFamilyIndices;
	ImgnWindow& win = ImgnWindow::GetInstance();
	VkInstance instance = nullptr;
	VkPhysicalDevice physicalDevice = nullptr;
	VkDevice device = nullptr;
	VkQueue graphicsQueue = nullptr;
	VkQueue presentQueue = nullptr;
	VkPipelineCache pipelineCache = nullptr;
	VkDescriptorPool descriptorPool = nullptr;
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
	VkCommandPool commandPool = nullptr;

	VulkanContext& operator=(const VulkanContext& pCtx)
	{
		if (this == &pCtx) return *this;

		//win = pCtx.win;
		instance = pCtx.instance;
		physicalDevice = pCtx.physicalDevice;
		device = pCtx.device;
		graphicsQueue = pCtx.graphicsQueue;
		presentQueue = pCtx.presentQueue;
		pipelineCache = pCtx.pipelineCache;
		descriptorPool = pCtx.descriptorPool;
		pipeline = pCtx.pipeline;
		pipelineLayout = pCtx.pipelineLayout;
		commandPool = pCtx.commandPool;
		queueFamilyIndices = pCtx.queueFamilyIndices;

		return *this;
	}
};

inline static RETURN(VkCommandBuffer) StartCommandBuffer(VulkanContext pCtx)
{
	VkCommandBuffer commandBuffer;

	VkCommandBufferAllocateInfo commandBufferAllocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		//.pNext = ,
		.commandPool = pCtx.commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	vkAllocateCommandBuffers(pCtx.device, &commandBufferAllocateInfo, &commandBuffer);

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

static RETURN(void) EndCommandBuffer(VulkanContext pCtx, VkCommandBuffer pCommandBuffer)
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

	vkQueueSubmit(pCtx.graphicsQueue, 1, &submitInfo, nullptr);
	vkQueueWaitIdle(pCtx.graphicsQueue);
	vkFreeCommandBuffers(pCtx.device, pCtx.commandPool, 1, &pCommandBuffer);

	return {};
}
