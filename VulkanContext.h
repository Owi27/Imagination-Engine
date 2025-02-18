#pragma once
using namespace Microsoft::WRL;

class VulkanContext
{
	static inline VulkanContext* _vulkanContext = nullptr;
	GVulkanSurface _vulkanSurface;

	//vulkan
	VkDevice _device;
	VkPhysicalDevice _physicalDevice;
	VkInstance _instance;
	VkCommandPool _commandPool;
	VkQueue _graphicsQueue;
	VkSwapchainKHR _swapchain;
	VkRenderPass _renderPass;
	VkFramebuffer _frameBuffer;

	unsigned int _maxFramesInFlight = 0;
	float _aspectRatio;

	//dxc
	ComPtr<IDxcCompiler3> _compiler;
	ComPtr<IDxcUtils> _utils;
	ComPtr<IDxcIncludeHandler> _includeHandler;

	VulkanContext() = default;
	VulkanContext(GWindow win);
	~VulkanContext() = default;

public:

	static VulkanContext* GetInst()
	{
		if (!_vulkanContext) _vulkanContext = new VulkanContext();

		return _vulkanContext;
	}

	static VulkanContext* GetInst(GWindow win)
	{
		if (!_vulkanContext) _vulkanContext = new VulkanContext(win);

		return _vulkanContext;
	}

	VkDevice GetDevice() const { return _device; }
	VkPhysicalDevice GetPhysicalDevice() const { return _physicalDevice; }
	VkInstance GetInstance() const { return _instance; }
	VkCommandPool GetCommandPool() const { return _commandPool; }
	VkQueue GetGraphicsQueue() const { return _graphicsQueue; }
	VkSwapchainKHR GetSwapchain() const { return _swapchain; }	
	VkRenderPass GetRenderPass() const { return _renderPass; }
	VkFramebuffer GetFrameBuffer(int idx);

	ComPtr<IDxcCompiler3> GetCompiler() const { return _compiler; }
	ComPtr<IDxcUtils> GetUtils() const { return _utils; }
	ComPtr<IDxcIncludeHandler> GetIncludeHandler() const { return _includeHandler; }
};

