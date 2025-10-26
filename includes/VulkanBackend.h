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

struct PipelineBuilder
{
	PipelineBuilder(VulkanContext& vk)
	{
		_vk = std::make_unique<VulkanContext>(vk);
	}

	~PipelineBuilder() = default;


	virtual VkPipeline BuildPipeline() = 0;

protected:
	std::unique_ptr<VulkanContext> _vk;
};

enum class PipelineFormat
{
	FLOAT = VK_FORMAT_R32_SFLOAT,
	FLOAT2 = VK_FORMAT_R32G32_SFLOAT,
	FLOAT3 = VK_FORMAT_R32G32B32_SFLOAT,
	FLOAT4 = VK_FORMAT_R32G32B32A32_SFLOAT,
	HFLOAT = VK_FORMAT_R16_SFLOAT,
	HFLOAT2 = VK_FORMAT_R16G16_SFLOAT,
	HFLOAT3 = VK_FORMAT_R16G16B16_SFLOAT,
	HFLOAT4 = VK_FORMAT_R16G16B16A16_SFLOAT,
	INT = VK_FORMAT_R32_SINT,
	INT2 = VK_FORMAT_R32G32_SINT,
	INT3 = VK_FORMAT_R32G32B32_SINT,
	INT4 = VK_FORMAT_R32G32B32A32_SINT,
	UINT = VK_FORMAT_R32_UINT,
	UINT2 = VK_FORMAT_R32G32_UINT,
	UINT3 = VK_FORMAT_R32G32B32_UINT,
	UINT4 = VK_FORMAT_R32G32B32A32_UINT,
	COLOR = VK_FORMAT_R8G8B8A8_UNORM
};

struct VertexInputDescription
{
	unsigned binding;
	unsigned location;
	unsigned stride;
	PipelineFormat format;
	unsigned offset;
};

enum class Topology
{
	POINT = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
	LINE_LIST = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
	LINE_STRIP = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	TRIANGLE_LIST = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	TRIANGLE_STRIPVK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
};

enum class PolygonMode
{
	FILL = VK_POLYGON_MODE_FILL,
	LINE = VK_POLYGON_MODE_LINE
};

struct RasterStateInfo
{
	PolygonMode polygonMode;
};

struct GraphicsPipelineBuilder : PipelineBuilder
{
	GraphicsPipelineBuilder(VulkanContext& vk) : PipelineBuilder(vk) 
	{
		VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = false
		};

		_pipelineInputAssemblyStateCreateInfo = pipelineInputAssemblyStateCreateInfo;
	}

	~GraphicsPipelineBuilder() = default;

	GraphicsPipelineBuilder& AddShaders(std::vector<class Shader> shaders);
	GraphicsPipelineBuilder& AddVertexBindingDescriptions(std::vector<VertexInputDescription> inputDescriptions);
	GraphicsPipelineBuilder& BuildTessellationState(unsigned patchControlPoints);
	GraphicsPipelineBuilder& BuildViewportState(unsigned width, unsigned height);
	GraphicsPipelineBuilder& SetTopology(Topology topology);
	GraphicsPipelineBuilder& SetRasterizationState(RasterStateInfo rasterStateInfo);

	VkPipeline BuildPipeline() override;

private:
	VkPipelineLayout _pipelineLayout;
	std::vector<VkPipelineShaderStageCreateInfo> _pipelineShaderStageCreateInfos;
	VkPipelineVertexInputStateCreateInfo _pipelineVertexInputStateCreateInfo;
	VkPipelineInputAssemblyStateCreateInfo _pipelineInputAssemblyStateCreateInfo;
	VkPipelineTessellationStateCreateInfo _pipelineTessellationStateCreateInfo;
	VkPipelineViewportStateCreateInfo _pipelineViewportStateCreateInfo;
	VkPipelineRasterizationStateCreateInfo _pipelineRasterizationStateCreateInfo;
	VkPipelineMultisampleStateCreateInfo _pipelineMultisampleStateCreateInfo;
	VkPipelineDepthStencilStateCreateInfo _pipelineDepthStencilStateCreateInfo;
	VkPipelineColorBlendStateCreateInfo _pipelineColorBlendStateCreateInfo;
	VkPipelineDynamicStateCreateInfo _pipelineDynamicStateCreateInfo;
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

