#pragma once
using namespace Microsoft::WRL;

struct PipelineDescription;
class Buffer;
class Texture;
class GuiContext;

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
	VkSampler _colorSampler;

	VkSemaphore _presentComplete;
	VkPresentInfoKHR _presentInfo;

	VkSurfaceFormatKHR _swapchainFormat = { .format = VK_FORMAT_B8G8R8A8_UNORM, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	unsigned int _maxFramesInFlight = 0, _width, _height, _currentFrame = 0, _currentImage = 0;;
	float _aspectRatio;

	bool _swapchainImagesInit = false;

	std::vector<VkFence> _fences;
	std::vector<VkSemaphore> _presentCompleteSemaphores;
	VkSubmitInfo _submitInfo;
	VkPipelineStageFlags _submitStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	std::vector<VkCommandBuffer> _swapchainCommandBuffers;

	std::vector<std::unique_ptr<Texture>> _currentSwapchainTextures;

	bool _firstFrame = true;

	//dxc
	ComPtr<IDxcCompiler3> _compiler;
	ComPtr<IDxcUtils> _utils;
	ComPtr<IDxcIncludeHandler> _includeHandler;

	PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR;
	PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR;


	GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE _windowHandle;
	//PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2KHR;



public:
	std::vector<std::function<void(VkCommandBuffer&)>> forwardCalls;
	std::unique_ptr<GuiContext> _guiContext;

	VulkanContext()
	{

	}

	VulkanContext(GWindow& win)
	{
		win.GetWindowHandle(_windowHandle);
#ifndef NDEBUG
		std::vector<const char*> debugLayers =
		{
			"VK_LAYER_KHRONOS_validation"
		};

		std::vector<const char*> deviceExt =
		{
			"VK_KHR_dynamic_rendering",
			VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME,
			VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME
		};

		if (+_vulkanSurface.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT | GW::GRAPHICS::TRIPLE_BUFFER | GW::GRAPHICS::BINDLESS_SUPPORT, debugLayers.size(), debugLayers.data(), 0, nullptr, deviceExt.size(), deviceExt.data(), false))
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

			for (size_t i = 0; i < _maxFramesInFlight; i++)
			{
				VkCommandBuffer commandBuffer;
				_vulkanSurface.GetCommandBuffer(i, (void**)&commandBuffer);
				_swapchainCommandBuffers.push_back(commandBuffer);
			}

			_fences.resize(_maxFramesInFlight);
			_presentCompleteSemaphores.resize(_maxFramesInFlight);

			VkSemaphoreCreateInfo semaphoreCreateInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			VkFenceCreateInfo fenceCreateInfo = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };

			for (size_t i = 0; i < _maxFramesInFlight; i++)
			{
				vkCreateFence(_device, &fenceCreateInfo, nullptr, &_fences[i]);
				vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_presentCompleteSemaphores[i]);
			}

			//submit info
			_submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
			_submitInfo.pWaitDstStageMask = &_submitStageFlags;
			_submitInfo.waitSemaphoreCount = 1;
			_submitInfo.signalSemaphoreCount = 1;

			_presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

			//dxc
			DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
			DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
			_utils->CreateDefaultIncludeHandler(&_includeHandler);
			std::filesystem::create_directories("Shaders/SPV");

			//for (size_t i = 0; i < _maxFramesInFlight; i++)
			//{
			//	VkImage swapchainImage;
			//	VkImageView swapchainImageView;
			//	VkFormat swapchainFormat;

			//	_vulkanSurface.GetSwapchainImage(i, (void**)&swapchainImage);
			//	_vulkanSurface.GetSwapchainView(i, (void**)&swapchainImageView);

			//	//GvkHelper::transition_image_layout(_device, _commandPool, _graphicsQueue, 1, swapchainImage, VK_FORMAT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			//}
		}

		//create sampler
		VkSamplerCreateInfo samplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; //todo try repeat?
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.maxAnisotropy = 1.f;
		samplerCreateInfo.maxLod = 1.f;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minLod = 0;
		samplerCreateInfo.mipLodBias = 0;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		vkCreateSampler(_device, &samplerCreateInfo, nullptr, &_colorSampler);

		vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)vkGetInstanceProcAddr(_instance, "vkCmdBeginRenderingKHR");
		vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetInstanceProcAddr(_instance, "vkCmdEndRenderingKHR");
		//	vkCmdPipelineBarrier2KHR = (PFN_vkCmdPipelineBarrier2KHR)vkGetInstanceProcAddr(_instance, "vkCmdPipelineBarrier2KHR");
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
	void SubmitQueue(VkSemaphore& prevSemaphore, VkSemaphore& currSemaphore, VkCommandBuffer& commandBuffers, VkFence fence = nullptr);
	void PresentInfo(VkSemaphore& semaphore);

	void MB(VkCommandBuffer& commandBuffer);

	unsigned FindMemoryType(unsigned typeFilter, VkMemoryPropertyFlags properties);

	VkDevice GetDevice() const { return _device; }
	VkPhysicalDevice GetPhysicalDevice() const { return _physicalDevice; }
	VkInstance GetInstance() const { return _instance; }
	VkCommandPool GetCommandPool() const { return _commandPool; }
	VkQueue GetGraphicsQueue() const { return _graphicsQueue; }
	VkSwapchainKHR& GetSwapchain() { return _swapchain; }
	VkSampler GetSampler() const { return _colorSampler; }
	VkRenderPass GetRenderPass() const { return _renderPass; }
	VkFramebuffer GetFrameBuffer(int idx);

	VkFence& GetCurrentFence() { return _fences[_currentFrame]; }

	VkSemaphore& GetSemaphore() { return _presentCompleteSemaphores[_currentFrame]; }

	VkFormat GetSwapchainFormat() const { return _swapchainFormat.format; }

	void CreateSwapchainTextures();

	VkCommandBuffer& GetCurrentCommandBuffer();

	ComPtr<IDxcCompiler3> GetCompiler() const { return _compiler; }
	ComPtr<IDxcUtils> GetUtils() const { return _utils; }
	ComPtr<IDxcIncludeHandler> GetIncludeHandler() const { return _includeHandler; }

	bool SwapchainImagesInitialized() const { return _swapchainImagesInit; }

	VkPipelineStageFlags GetPipelineStageFlags(VkImageLayout layout);
	VkAccessFlags GetAccessFlags(VkImageLayout layout);

	void* GetWindowHandle() { return _windowHandle.window; }


	//Texture& UploadTextureToGPU(tinygltf::Image glImage);

	unsigned GetWidth() const { return _width; }
	unsigned GetHeight() const { return _height; }
	unsigned GetAspectRatio() const { return _aspectRatio; }
	unsigned GetMaxFrames() const { return _maxFramesInFlight; }

	void ReadImGuiInputs(GInput& input);

	void TransitionImageLayout(VkCommandBuffer& commandBuffer, unsigned mipLevels, const VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout);
	void TransitionImageLayout(VkCommandBuffer& commandBuffer, unsigned mipLevels, unsigned layerCount, const VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout);

	void GetSwapchainImage(Texture* tex, unsigned idx);

	VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet& destinationSet, std::vector<VkDescriptorSetLayoutBinding>& layoutBindings, unsigned int destinationBinding, unsigned int arrayElement = 0) const;
	VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet& destinationSet, std::vector<VkDescriptorSetLayoutBinding>& layoutBindings, unsigned int destinationBinding, const VkDescriptorBufferInfo* descriptorBufferInfo, unsigned int arrayElement = 0);
	VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet& destinationSet, std::vector<VkDescriptorSetLayoutBinding>& layoutBindings, unsigned int destinationBinding, const VkDescriptorImageInfo* descriptorImageInfo, unsigned int arrayElement = 0);

	VkPipeline CreateGraphicsPipeline(struct PipelineDescription pipelineDescription, VkPipelineLayout& pipelineLayout, unsigned colorAttachmentCount = 1);
	VkPipeline CreateGuiGraphicsPipeline(struct PipelineDescription pipelineDescription, VkPipelineLayout& pipelineLayout, unsigned colorAttachmentCount = 1);

	VkCommandBuffer& Render(VkCommandBuffer& commandBuffer, std::vector<std::reference_wrapper<Texture>>& textures, Texture* depth, std::function<void(VkCommandBuffer&)> drawCalls);
	void RenderToSwapchain(Texture* depth, std::function<void(VkCommandBuffer&)> drawCalls, std::function<void(VkCommandBuffer&)> binds);
	//VkCommandBuffer& Render(VkCommandBuffer& commandBuffer, Texture* depth, std::function<void(VkCommandBuffer&)> drawCalls);
};