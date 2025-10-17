#pragma once
#include <vulkan/vulkan.h>
#include <expected>
#include <vector>
#include <Macros.h>
#include <optional>
//#define RETURN(t1, t2) std::expected<t1, t2> 

class VulkanBackend
{
	struct QueueFamilyIndices
	{
		std::optional<unsigned> graphicsFamily;

		bool IsComplete() { return graphicsFamily.has_value(); }
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
	VkQueue _graphicsQueue;
	VkSurfaceKHR _surface;


	RETURN(bool) CheckCompatibility(const char** instanceExtensions, const char** deviceExtensions);
	RETURN(bool) CheckLayerSupport();

	RETURN(void) PickPhysicalDevice();
	RETURN(bool) IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice);
	RETURN(QueueFamilyIndices) FindQueueFamilies(VkPhysicalDevice physicalDevice);

	RETURN(void) CreateDevice();
	RETURN(void) CreateSurface();

	RETURN(void) CreateInstance(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions);

public:
	VulkanBackend()
	{
#ifdef DEBUG
		_instanceLayers.emplace_back("VK_LAYER_KHRONOS_validation");
#endif
	}

	~VulkanBackend()
	{
		vkDestroyInstance(_instance, nullptr);

	}

	std::expected<bool, const char*> Init(unsigned layerCount = 0, const char** layers = nullptr, unsigned extensionCount = 0, const char** extensions = nullptr, unsigned dExtensionCount = 0, const char** dExtensions = nullptr);
	//std::expected<bool, const char*> Init(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions);
};

