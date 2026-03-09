#pragma once
#include "VulkanCtx.h"
#include "Swapchain.h"

class VkBackend
{
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
		VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
		VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
		VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
		VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME
	};

	Swapchain _swapchain;

	bool CheckCompatibility(const char** pInstanceExtensions, const char** pDeviceExtensions);
	bool CheckLayerSupport();
	void PickPhysicalDevice();
	bool IsPhysicalDeviceSuitable(VkPhysicalDevice pPhysicalDevice);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice pPhysicalDevice);

	void CreateInstance(unsigned pLayerCount, const char** pLayers, unsigned pExtensionCount, const char** pExtensions);
	void CreateDevice();
	void CreateSurface();
	void CreateCommandPool();
	void CreateSemaphoreAndFences();
	void CreateGraphicsPipeline();
	void CreateDefaultSampler();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice pPhysicalDevice);

public:
	VkBackend() /*Constructor*/
	{
	}

	~VkBackend() /*Destructor*/
	{
	}

	VkBackend(const VkBackend& pOther) /*Copy Constructor*/
	{
	}

	VkBackend& operator=(const VkBackend& pOther) /*Copy Assignment Operator*/
	{
		if (this != &pOther)
		{
		}

		return *this;
	}

	VkBackend(VkBackend&& pOther) noexcept /*Move Constructor*/
	{
	}

	VkBackend& operator=(VkBackend&& pOther) noexcept /*Move Assignment Operator*/
	{
		if (this != &pOther)
		{
		}

		return *this;
	}

	void Init(unsigned layerCount = 0, const char** layers = nullptr, unsigned extensionCount = 0, const char** extensions = nullptr, unsigned dExtensionCount = 0, const char** dExtensions = nullptr);
	VkCommandBuffer StartFrame();
	void EndFrame(VkCommandBuffer commandBuffer);

};