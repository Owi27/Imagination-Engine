#include "VulkanBackend.h"



RETURN(bool, const char*) VulkanBackend::CheckCompatibility(const char** instanceExtensions, const char** deviceExtensions)
{
	const char* use_surface = VK_KHR_SURFACE_EXTENSION_NAME;
	const char* use_swapchain = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	//	const char* platform_surface = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
	const char* platform_device = nullptr;

	return false;
}

RETURN(bool, const char*) VulkanBackend::CheckLayerSupport()
{
	unsigned layerCount;

	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : _instanceLayers) 
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) 
		{
			if (strcmp(layerName, layerProperties.layerName) == 0) 
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound) return false;
	}

	return true;
}

RETURN(void) VulkanBackend::PickPhysicalDevice()
{
	unsigned deviceCount = 0;
	vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

	if (deviceCount == 0)  return std::unexpected("VulkanBackend.cpp | PickPhysicalDevice() | physical device not found");

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(_instance, &deviceCount, physicalDevices.data());

	for (const auto& device : physicalDevices) 
	{
		if (*IsPhysicalDeviceSuitable(device))
		{
			_physicalDevice = device;
			break;
		}
	}

	if (_physicalDevice == nullptr) return std::unexpected("VulkanBackend.cpp | PickPhysicalDevice() | no suitable physical device");

}

RETURN(bool) VulkanBackend::IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice)
{
	//VkPhysicalDeviceProperties physicalDeviceProperties;
	//vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	//VkPhysicalDeviceFeatures physicalDeviceFeatures;
	//vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

	QueueFamilyIndices indices = *FindQueueFamilies(physicalDevice);

	return indices.IsComplete();
}

RETURN(VulkanBackend::QueueFamilyIndices) VulkanBackend::FindQueueFamilies(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices indices;

	unsigned queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) 
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;
		if (indices.IsComplete()) break;
		i++;
	}

	return indices;
}

RETURN(void) VulkanBackend::CreateDevice()
{
	QueueFamilyIndices indices = *FindQueueFamilies(_physicalDevice);
	
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;

	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures;

	VkDeviceCreateInfo createInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCreateInfo,
		.enabledLayerCount = _enableValidationLayers ? (unsigned)_instanceLayers.size() : 0,
		.ppEnabledLayerNames = _enableValidationLayers ? _instanceLayers.data() : nullptr,
		.enabledExtensionCount = 0,
		.pEnabledFeatures = &deviceFeatures,
	};

	if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device)) return std::unexpected("VulkanBackend.cpp | CreateDevice() | vkCreateDevice");

	vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
}

RETURN(void) VulkanBackend::CreateSurface()
{
}

std::expected<void, const char*> VulkanBackend::CreateInstance(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions)
{  
	if (_enableValidationLayers && !*CheckLayerSupport()) return std::unexpected("VulkanBackend.cpp | CreateInstance() | instance layers requested, but not available");

	VkApplicationInfo applicationInfo
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Imagination",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Imagination",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_3
	};

	VkInstanceCreateInfo instanceCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &applicationInfo,
		.enabledLayerCount = (unsigned)_instanceLayers.size(),
		.ppEnabledLayerNames = _instanceLayers.data(),
		.enabledExtensionCount = (unsigned)_instanceExtensions.size(),
		.ppEnabledExtensionNames = _instanceExtensions.data()
	};

	if (vkCreateInstance(&instanceCreateInfo, nullptr, &_instance)) return std::unexpected("VulkanBackend.cpp | CreateInstance() | vkCreateInstance");

}

std::expected<bool, const char*> VulkanBackend::Init(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions, unsigned dExtensionCount, const char** dExtensions)
{
	_instanceLayers.resize(_instanceLayers.size() + layerCount);
	for (size_t i = _instanceLayers.size() - layerCount; i < layerCount; i++) _instanceLayers[i] = layers[i];

	_instanceExtensions.resize(_instanceExtensions.size() + extensionCount);
	for (size_t i = 0; i < extensionCount; i++) _instanceExtensions[i] = extensions[i];

	_deviceExtensions.resize(_deviceExtensions.size() + dExtensionCount);
	for (size_t i = 0; i < dExtensionCount; i++) _deviceExtensions[i] = dExtensions[i];


	CreateInstance(_instanceLayers.size(), _instanceLayers.data(), _instanceExtensions.size(), _instanceExtensions.data());


	return true;
}