#pragma once
#include <vulkan/vulkan.h>
#include <expected>
#include <vector>
#include <Macros.h>
#include <optional>

struct VulkanContext
{
	VkInstance _instance = nullptr;
	VkPhysicalDevice _physicalDevice = nullptr;
	VkDevice _device = nullptr;
	VkQueue _graphicsQueue = nullptr;
	unsigned _graphicsQueueFamily = 0;
	VkQueue _presentQueue = nullptr;
	unsigned _presentQueueFamily = 0;
	VkPipelineCache _pipelineCache = nullptr;
	VkDescriptorPool _descriptorPool = nullptr;
};

class VulkanBackend
{
	struct QueueFamilyIndices
	{
		std::optional<unsigned> graphicsFamily;
		std::optional<unsigned> presentFamily;

		bool IsComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

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
#ifdef DEBUG
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
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME
	};

	//vulkan
	VkInstance _instance = nullptr;
	VkDevice _device = nullptr;
	VkPhysicalDevice _physicalDevice = nullptr;
	VkQueue _graphicsQueue, _presentQueue;
	VkSurfaceKHR _surface;
	VkSwapchainKHR _swapchain;
	std::vector<VkImage> _swapchainImages;
	VkFormat _swapchainImageFormat;
	VkExtent2D _swapchainExtent;
	std::vector<VkImageView> _swapchainImageViews;



	RETURN(bool) CheckCompatibility(const char** instanceExtensions, const char** deviceExtensions);
	RETURN(bool) CheckLayerSupport();

	RETURN(void) PickPhysicalDevice();
	RETURN(bool) IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice);
	RETURN(QueueFamilyIndices) FindQueueFamilies(VkPhysicalDevice physicalDevice);

	

	RETURN(void) CreateDevice();
	RETURN(void) CreateSurface();
	RETURN(void) CreateSwapchain();
	RETURN(void) CreateSwapchainImageViews();
	RETURN(void) CreateGraphicsPipeline();

	RETURN(bool) CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
	RETURN(SwapchainSupportDetails) QuerySwapchainSupport(VkPhysicalDevice physicalDevice);
	RETURN(VkSurfaceFormatKHR) ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	RETURN(VkPresentModeKHR) ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	RETURN(VkExtent2D) ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

	RETURN(void) CreateInstance(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions);

public:
	VulkanBackend()
	{
	}

	~VulkanBackend()
	{
		for (auto imageView : _swapchainImageViews)
		{
			vkDestroyImageView(_device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(_device, _swapchain, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyInstance(_instance, nullptr);

	}

	RETURN(bool) Init(unsigned layerCount = 0, const char** layers = nullptr, unsigned extensionCount = 0, const char** extensions = nullptr, unsigned dExtensionCount = 0, const char** dExtensions = nullptr);
	//std::expected<bool, const char*> Init(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions);
};

