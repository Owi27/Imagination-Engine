#pragma once
#include <vulkan/vulkan.h>
#include <expected>
#include <vector>
#include <Macros.h>
#include "Shader.h"
#include "Resource.hpp"



class VulkanBackend
{
	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		std::vector<VkPresentModeKHR> presentModes;
	};


#ifdef NDEBUG
	const bool _enableValidationLayers = false;
#else
	const bool _enableValidationLayers = true;
#endif

	std::vector<const char*> _instanceLayers =
	{
#ifdef NDEBUG
#else
		"VK_LAYER_KHRONOS_validation"
#endif
	};

	std::vector<const char*> _instanceExtensions =
	{
		"VK_KHR_surface",
		"VK_KHR_win32_surface"
	};

	std::vector<const char*> _deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		//VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
		//VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME,
		"VK_KHR_pipeline_executable_properties",
		VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
		VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME
	};

	//vulkan
	//VkInstance _instance = nullptr;
	//VkDevice _device = nullptr;
	//VkPhysicalDevice _physicalDevice = nullptr;
	//VkQueue _graphicsQueue, _presentQueue;
	VkSurfaceKHR _surface;
	VkSwapchainKHR _swapchain;
	//std::vector<VkImage> _swapchainImages;
	VkFormat _swapchainImageFormat;
	VkExtent2D _swapchainExtent;
	//std::vector<VkImageView> _swapchainImageViews;
	std::vector<VkCommandBuffer> _swapchainCommandBuffers;
	std::vector<VkSemaphore> _imageAvailableSemaphores, _renderFinishedSemaphores;
	std::vector<VkFence> _renderingFences;


	bool _frameLocked, _initialFrames = true;
	unsigned _currentFrame = 0, _targetFrame = 0, _maxFrames, _frame = 0;


	RETURN(bool) CheckCompatibility(const char** instanceExtensions, const char** deviceExtensions);
	RETURN(bool) CheckLayerSupport();

	RETURN(void) PickPhysicalDevice();
	RETURN(bool) IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice);
	RETURN(VulkanContext::QueueFamilyIndices) FindQueueFamilies(VkPhysicalDevice physicalDevice);

	//RETURN(VkImage) CreateImage

	RETURN(void) CreateInstance(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions);
	RETURN(void) CreateDevice();
	RETURN(void) CreateSurface();
	RETURN(void) CreateSwapchain();
	RETURN(void) CreateSwapchainImageViews();
	void CreateGraphicsPipeline();


	void InitDepthTexture();
	RETURN(bool) CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
	RETURN(SwapchainSupportDetails) QuerySwapchainSupport(VkPhysicalDevice physicalDevice);
	RETURN(VkSurfaceFormatKHR) ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	RETURN(VkPresentModeKHR) ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	RETURN(VkExtent2D) ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);


public:
	VulkanContext _vk;
	//Swapchain swapchainInfo;

	VulkanBackend()
	{
	}

	~VulkanBackend()
	{
		//for (* image : _swapchainImages)
		//{
		//	delete image;
		//	//vkDestroyImageView(_vk.device, imageView, nullptr);
		//}

		vkDestroyPipeline(_vk.device, _vk.pipeline, nullptr);
		vkDestroyPipelineLayout(_vk.device, _vk.pipelineLayout, nullptr);
		vkDestroySwapchainKHR(_vk.device, _swapchain, nullptr);
		vkDestroySurfaceKHR(_vk.instance, _surface, nullptr);
		vkDestroyInstance(_vk.instance, nullptr);
	}

	RETURN(bool) Init(unsigned layerCount = 0, const char** layers = nullptr, unsigned extensionCount = 0, const char** extensions = nullptr, unsigned dExtensionCount = 0, const char** dExtensions = nullptr);

	RETURN(VkCommandBuffer) StartFrame();
	RETURN(void) EndFrame(VkCommandBuffer commandBuffer);

	void GetCurrentFrame(Texture& pSwap)
	{
		pSwap.Swapchain(_vk.swapchainImages[_currentFrame], _vk.swapchainImageViews[_currentFrame], _vk.swapchainFormat, { _vk.swapchainExtent.width, _vk.swapchainExtent.height, 1 }, ImageAspect::COLOR);
		pSwap.imageLayout = ImageLayout::UNDEFINED;
		pSwap.TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
	}
};
