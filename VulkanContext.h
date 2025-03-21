#pragma once
using namespace Microsoft::WRL;

struct PipelineDescription;
class Buffer;
class Texture;

class VulkanContext
{
	static inline std::unique_ptr<VulkanContext> _vulkanContext = nullptr;
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

	unsigned int _maxFramesInFlight = 0, _width, _height;
	float _aspectRatio;

	//dxc
	ComPtr<IDxcCompiler3> _compiler;
	ComPtr<IDxcUtils> _utils;
	ComPtr<IDxcIncludeHandler> _includeHandler;

	PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR;
	PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR;

public:
	VulkanContext()
	{

	}

	VulkanContext(GWindow& win)
	{
#ifndef NDEBUG
		std::vector<const char*> debugLayers =
		{
			"VK_LAYER_KHRONOS_validation"
		};

		std::vector<const char*> deviceExt =
		{
			"VK_KHR_dynamic_rendering",
			VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME
		};

		if (+_vulkanSurface.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT | GW::GRAPHICS::TRIPLE_BUFFER, debugLayers.size(), debugLayers.data(), 0, nullptr, deviceExt.size(), deviceExt.data(), false))
#else
		if (+_vulkanSurface.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT | GW::GRAPHICS::TRIPLE_BUFFER))
#endif
		{
			win.GetClientWidth(_width);
			win.GetClientHeight(_height);
			_vulkanSurface.GetDevice((void**)&_device);
			_vulkanSurface.GetPhysicalDevice((void**)&_physicalDevice);
			_vulkanSurface.GetInstance((void**)&_instance);
			_vulkanSurface.GetGraphicsQueue((void**)&_graphicsQueue);
			_vulkanSurface.GetCommandPool((void**)&_commandPool);
			_vulkanSurface.GetGraphicsQueue((void**)&_graphicsQueue);
			_vulkanSurface.GetSwapchain((void**)&_swapchain);
			_vulkanSurface.GetSwapchainImageCount(_maxFramesInFlight);
			_vulkanSurface.GetAspectRatio(_aspectRatio);

			//dxc
			DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
			DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
			_utils->CreateDefaultIncludeHandler(&_includeHandler);
			std::filesystem::create_directories("Shaders/SPV");

			for (size_t i = 0; i < _maxFramesInFlight; i++)
			{
				VkImage swapchainImage;
				VkImageView swapchainImageView;
				VkFormat swapchainFormat;

				_vulkanSurface.GetSwapchainImage(i, (void**)&swapchainImage);
				_vulkanSurface.GetSwapchainView(i, (void**)&swapchainImageView);

				GvkHelper::transition_image_layout(_device, _commandPool, _graphicsQueue, 1, swapchainImage, VK_FORMAT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			}
		}

		vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)vkGetInstanceProcAddr(_instance, "vkCmdBeginRenderingKHR");
		vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetInstanceProcAddr(_instance, "vkCmdEndRenderingKHR");
	}

	VulkanContext(const VulkanContext&) = delete;
	VulkanContext& operator=(const VulkanContext&) = delete;

	~VulkanContext()
	{
	}

	static VulkanContext* GetInst()
	{
		if (!_vulkanContext) return nullptr;

		return _vulkanContext.get();
	}

	static VulkanContext* GetInst(GWindow win)
	{
		if (!_vulkanContext) _vulkanContext = std::make_unique<VulkanContext>(win);

		return _vulkanContext.get();
	}

	void StartFrame();
	void EndFrame();

	VkDevice GetDevice() const { return _device; }
	VkPhysicalDevice GetPhysicalDevice() const { return _physicalDevice; }
	VkInstance GetInstance() const { return _instance; }
	VkCommandPool GetCommandPool() const { return _commandPool; }
	VkQueue GetGraphicsQueue() const { return _graphicsQueue; }
	VkSwapchainKHR& GetSwapchain() { return _swapchain; }
	VkRenderPass GetRenderPass() const { return _renderPass; }
	VkFramebuffer GetFrameBuffer(int idx);


	ComPtr<IDxcCompiler3> GetCompiler() const { return _compiler; }
	ComPtr<IDxcUtils> GetUtils() const { return _utils; }
	ComPtr<IDxcIncludeHandler> GetIncludeHandler() const { return _includeHandler; }

	unsigned GetWidth() const { return _width; }
	unsigned GetHeight() const { return _height; }
	unsigned GetAspectRatio() const { return _aspectRatio; }

	VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet& destinationSet, std::vector<VkDescriptorSetLayoutBinding>& layoutBindings, unsigned int destinationBinding, unsigned int arrayElement = 0) const;
	VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet& destinationSet, std::vector<VkDescriptorSetLayoutBinding>& layoutBindings, unsigned int destinationBinding, const VkDescriptorBufferInfo* descriptorBufferInfo, unsigned int arrayElement = 0);

	VkPipeline CreateGraphicsPipeline(struct PipelineDescription pipelineDescription, VkPipelineLayout& pipelineLayout, unsigned colorAttachmentCount = 1);

	VkCommandBuffer& Render(VkCommandBuffer& commandBuffer, std::vector<Texture*>& textures, Texture& depth, std::function<void(VkCommandBuffer&)> drawCalls);
};