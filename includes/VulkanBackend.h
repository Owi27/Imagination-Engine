#pragma once
#include <vulkan/vulkan.h>
#include <expected>
#include <vector>
#include <Macros.h>
#include <optional>
#include <IWindow.h>

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
	COLOR = VK_FORMAT_R8G8B8A8_UNORM,
	UNDEFINED = VK_FORMAT_UNDEFINED
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

enum class CullMode
{
	FRONT = VK_CULL_MODE_FRONT_BIT,
	BACK = VK_CULL_MODE_BACK_BIT,
	NONE = VK_CULL_MODE_NONE
};

enum class FrontFace
{
	COUNTER_CLOCKWISE = VK_FRONT_FACE_COUNTER_CLOCKWISE,
	CLOCKWISE = VK_FRONT_FACE_CLOCKWISE
};

struct RasterStateInfo
{
	PolygonMode polygonMode;
	CullMode cullMode;
	FrontFace frontFace;
};

enum class BlendFactor
{
	FULL = VK_BLEND_FACTOR_ONE,
	NONE = VK_BLEND_FACTOR_ZERO,
};

enum class BlendOperation
{
	ADD = VK_BLEND_OP_ADD,
	SUBTRACT = VK_BLEND_OP_SUBTRACT,
	MIN = VK_BLEND_OP_MIN,
	MAX = VK_BLEND_OP_MAX
};

enum class ColorComponent
{
	R = VK_COLOR_COMPONENT_R_BIT,
	G = VK_COLOR_COMPONENT_G_BIT,
	B = VK_COLOR_COMPONENT_B_BIT,
	A = VK_COLOR_COMPONENT_A_BIT
};

struct PipelineAttachment
{
	bool blend = false;

	BlendFactor colorSource, colorDestination;
	BlendOperation colorOperation = BlendOperation::ADD;

	BlendFactor alphaSource, alphaDestination;
	BlendOperation alphaOperation = BlendOperation::ADD;

	VkColorComponentFlags writeMask = (VkColorComponentFlags)ColorComponent::R | (VkColorComponentFlags)ColorComponent::G | (VkColorComponentFlags)ColorComponent::B | (VkColorComponentFlags)ColorComponent::A;
};

struct RenderingInfo
{
	std::vector<PipelineFormat> colorAttachmentFormats;
	PipelineFormat depthStencilFormat = PipelineFormat::UNDEFINED;
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

		VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.patchControlPoints = 0,
		};

		VkViewport viewport
		{
			.x = 0,
			.y = 0,
			.width = (float)ImgnWindow::GetInstance().GetWidth(),
			.height = (float)ImgnWindow::GetInstance().GetHeight(),
			.minDepth = 0,
			.maxDepth = 1,
		};

		VkRect2D scissor
		{
			.offset
			{
				.x = 0,
				.y = 0
			},
			.extent
			{
				.width = ImgnWindow::GetInstance().GetWidth(),
				.height = ImgnWindow::GetInstance().GetHeight()
			}
		};

		VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.viewportCount = 1,
			.pViewports = &viewport,
			.scissorCount = 1,
			.pScissors = &scissor,
		};

		VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo
		{
			//.pNext = ,
			//.flags = ,

			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.depthClampEnable = false,
			.rasterizerDiscardEnable = false,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_FRONT_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = false,
			.depthBiasConstantFactor = 0.0f,
			.depthBiasClamp = 0.0f,
			.depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.f,
		};

		VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = false,
			.minSampleShading = 1.f,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = false,
			.alphaToOneEnable = false,
		};

		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.depthTestEnable = false,
			.depthWriteEnable = false,
			.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
			.depthBoundsTestEnable = false,
			.stencilTestEnable = false,
			//.front = ,
			//.back = ,
			.minDepthBounds = 0.f,
			.maxDepthBounds = 1.f,
		};

		VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.logicOpEnable = false,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 0,
			.pAttachments = nullptr,
			.blendConstants
			{
				0.f, 0.f, 0.f, 0.f
			},
		};

		VkDynamicState dynamicState[2] =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.dynamicStateCount = 2,
			.pDynamicStates = dynamicState
		};

		VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
			//.pNext = ,
			//.viewMask = ,
			.colorAttachmentCount = 0,
			.pColorAttachmentFormats = nullptr,
			.depthAttachmentFormat = VK_FORMAT_UNDEFINED,
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
		};

		_pipelineInputAssemblyStateCreateInfo = pipelineInputAssemblyStateCreateInfo;
		_pipelineTessellationStateCreateInfo = pipelineTessellationStateCreateInfo;
		_pipelineViewportStateCreateInfo = pipelineViewportStateCreateInfo;
		_pipelineRasterizationStateCreateInfo = pipelineRasterizationStateCreateInfo;
		_pipelineMultisampleStateCreateInfo = pipelineMultisampleStateCreateInfo;
		_pipelineDepthStencilStateCreateInfo = pipelineDepthStencilStateCreateInfo;
		_pipelineColorBlendStateCreateInfo = pipelineColorBlendStateCreateInfo;
		_pipelineDynamicStateCreateInfo = pipelineDynamicStateCreateInfo;
		_pipelineRenderingCreateInfo = pipelineRenderingCreateInfo;
	}

	~GraphicsPipelineBuilder() = default;

	GraphicsPipelineBuilder& AddShaders(std::vector<class Shader> shaders);
	GraphicsPipelineBuilder& AddVertexBindingDescriptions(std::vector<VertexInputDescription> inputDescriptions);
	GraphicsPipelineBuilder& BuildTessellationState(unsigned patchControlPoints);
	GraphicsPipelineBuilder& BuildViewportState(unsigned width, unsigned height);
	GraphicsPipelineBuilder& SetTopology(Topology topology);
	GraphicsPipelineBuilder& SetRasterizationState(RasterStateInfo rasterStateInfo);
	GraphicsPipelineBuilder& AddDepthTest();
	GraphicsPipelineBuilder& AddDepthWrite();
	GraphicsPipelineBuilder& AddStencilTest();
	GraphicsPipelineBuilder& AddPipelineAttachments(std::vector<PipelineAttachment> pipelineAttachments);
	GraphicsPipelineBuilder& SetRenderingInfo(RenderingInfo renderingInfo);

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
	VkPipelineRenderingCreateInfoKHR _pipelineRenderingCreateInfo;
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

