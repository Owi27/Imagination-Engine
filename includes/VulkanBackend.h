#pragma once
#include <vulkan/vulkan.h>
#include <expected>
#include <vector>
#include <Macros.h>
#include <optional>

class VulkanBackend
{
	struct QueueFamilyIndices
	{
		std::optional<unsigned> graphicsFamily;
		std::optional<uint32_t> presentFamily;

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

	std::vector<const char*> _instanceLayers;
	std::vector<const char*> _instanceExtensions;
	std::vector<const char*> _deviceExtensions;

	//vulkan
	VkInstance _instance = nullptr;
	VkDevice _device = nullptr;
	VkPhysicalDevice _physicalDevice = nullptr;
	VkQueue _graphicsQueue, _presentQueue;
	VkSurfaceKHR _surface;
	VkSwapchainKHR _swapchain;
	std::vector<VkImage> _swapchainImages;
	VkFormat swapchainImageFormat;
	VkExtent2D swapchainExtent;
	std::vector<VkImageView> swapchainImageViews;



	RETURN(bool) CheckCompatibility(const char** instanceExtensions, const char** deviceExtensions);
	RETURN(bool) CheckLayerSupport();

	RETURN(void) PickPhysicalDevice();
	RETURN(bool) IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice);
	RETURN(QueueFamilyIndices) FindQueueFamilies(VkPhysicalDevice physicalDevice);

	RETURN(void) CreateDevice();
	RETURN(void) CreateSurface();
	RETURN(void) CreateSwapchain();

	RETURN(bool) CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
	RETURN(SwapchainSupportDetails) QuerySwapchainSupport(VkPhysicalDevice physicalDevice);
	RETURN(VkSurfaceFormatKHR) ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	RETURN(VkPresentModeKHR) ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	RETURN(VkExtent2D) ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

	RETURN(void) CreateInstance(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions);

public:
	VulkanBackend()
	{
#ifdef DEBUG
		_instanceLayers.emplace_back("VK_LAYER_KHRONOS_validation");
#endif

		_instanceExtensions.push_back("VK_KHR_surface");
		_instanceExtensions.push_back("VK_KHR_win32_surface");

		_deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	~VulkanBackend()
	{
		vkDestroySwapchainKHR(_device, _swapchain, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyInstance(_instance, nullptr);

	}

	RETURN(bool) Init(unsigned layerCount = 0, const char** layers = nullptr, unsigned extensionCount = 0, const char** extensions = nullptr, unsigned dExtensionCount = 0, const char** dExtensions = nullptr);
	//std::expected<bool, const char*> Init(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions);
};

