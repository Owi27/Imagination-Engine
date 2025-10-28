#pragma once
#include <vulkan/vulkan.h>
#include <expected>
#include <vector>
#include <Macros.h>
#include <optional>
#include <IWindow.h>
#include "Shader.h"

struct VulkanContext
{
	VkInstance instance = nullptr;
	VkPhysicalDevice physicalDevice = nullptr;
	VkDevice device = nullptr;
	VkQueue graphicsQueue = nullptr;
	unsigned graphicsQueueFamily = 0;
	VkQueue presentQueue = nullptr;
	unsigned presentQueueFamily = 0;
	VkPipelineCache pipelineCache = nullptr;
	VkDescriptorPool descriptorPool = nullptr;
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
};


//enum class ShaderType
//{
//	FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
//	VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
//	COMPUTE = VK_SHADER_STAGE_COMPUTE_BIT
//};
//
//struct Shader
//{
//	Shader() = default;
//
//	Shader(VkDevice device, const std::string& filename, ShaderType shaderType)
//	{
//		_device = device;
//		_shaderStageFlagBits = (VkShaderStageFlagBits)shaderType;
//
//		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
//		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
//		_utils->CreateDefaultIncludeHandler(&_includeHandler);
//
//		Attempt(Compile(filename, shaderType));
//	}
//
//	~Shader()
//	{
//		vkDestroyShaderModule(_device, _shaderModule, nullptr);
//		//delete[] data;
//	}
//
//	VkShaderModule GetShaderModule() const { return _shaderModule; }
//	VkShaderStageFlagBits GetShaderStageFlagBits() const { return _shaderStageFlagBits; }
//	std::string GetEntryPointName() const { return _entryPointName; }
//	RETURN(VkPipelineShaderStageCreateInfo) GetPipelineShaderStageCreateInfo() const;
//
//	void SetEntryPointName(const std::string& entryPointName) { _entryPointName = entryPointName; }
//
//private:
//	VkDevice _device;
//	VkShaderModule _shaderModule;
//	VkShaderStageFlagBits _shaderStageFlagBits;
//	VkPipelineShaderStageCreateInfo _pssci;
//	std::string _entryPointName = "main", _shaderString, _spvPath, _shader, _spv;
//	unsigned long long _shaderSize, _spvSize;
//	//std::string _shaderString, _spvPath, _newShader;
//	//char* data;
//
//	//dxc
//	ComPtr<IDxcCompiler3> _compiler;
//	ComPtr<IDxcUtils> _utils;
//	ComPtr<IDxcIncludeHandler> _includeHandler;
//
//	RETURN(std::string) ShaderAsString(const char* shaderFilePath);
//	RETURN(void) ReadSPVFile(const std::string& filename);
//	//RETURN(void) CreateShader();
//	RETURN(void) Compile(const std::string& filename, ShaderType shaderType);
//	RETURN(bool) Reload();
//	RETURN(void) CreateShaderModule();
//};

struct PipelineBuilder
{
	PipelineBuilder(VulkanContext& vk)
	{
		_vk = std::make_unique<VulkanContext>(vk);
	}

	~PipelineBuilder() = default;


	virtual RETURN(VkPipeline) BuildPipeline() = 0;

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
	SWAPCHAIN = VK_FORMAT_B8G8R8A8_SRGB,
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
	TRIANGLE_STRIP = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
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
	SOURCE_ALPHA = VK_BLEND_FACTOR_SRC_ALPHA,
	NEG_SOURCE_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
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
	bool blend = true;

	BlendFactor colorSource = BlendFactor::SOURCE_ALPHA, colorDestination = BlendFactor::NEG_SOURCE_ALPHA;
	BlendOperation colorOperation = BlendOperation::ADD;

	BlendFactor alphaSource = BlendFactor::FULL, alphaDestination = BlendFactor::NONE;
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
		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.vertexBindingDescriptionCount = 0,
			.pVertexBindingDescriptions = nullptr,
			.vertexAttributeDescriptionCount = 0,
			.pVertexAttributeDescriptions = nullptr
		};

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

		_viewport =
		{
			.x = 0,
			.y = 0,
			.width = (float)ImgnWindow::GetInstance().GetWidth(),
			.height = (float)ImgnWindow::GetInstance().GetHeight(),
			.minDepth = 0,
			.maxDepth = 1,
		};

		_scissor =
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
			.pViewports = &_viewport,
			.scissorCount = 1,
			.pScissors = &_scissor,
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
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
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

		_dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.dynamicStateCount = (unsigned)_dynamicStates.size(),
			.pDynamicStates = _dynamicStates.data()
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

		_pipelineVertexInputStateCreateInfo = pipelineVertexInputStateCreateInfo;
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

	GraphicsPipelineBuilder& AddShaders(std::vector<std::pair<std::string, ShaderType>> shaders);
	GraphicsPipelineBuilder& AddVertexBindingDescriptions(std::vector<VertexInputDescription> inputDescriptions);
	GraphicsPipelineBuilder& BuildTessellationState(unsigned patchControlPoints);
	GraphicsPipelineBuilder& BuildViewportState(unsigned width, unsigned height);
	GraphicsPipelineBuilder& SetTopology(Topology topology);
	GraphicsPipelineBuilder& SetRasterizationState(RasterStateInfo rasterStateInfo);
	GraphicsPipelineBuilder& AddDepthTest();
	GraphicsPipelineBuilder& AddDepthWrite();
	GraphicsPipelineBuilder& AddStencilTest();
	GraphicsPipelineBuilder& AddPipelineAttachments(std::vector<PipelineAttachment> pipelineAttachments);
	GraphicsPipelineBuilder& AddDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
	GraphicsPipelineBuilder& AddPushConstantRange(VkPushConstantRange pushConstantRange);
	GraphicsPipelineBuilder& SetRenderingInfo(RenderingInfo renderingInfo);

	RETURN(VkPipeline) BuildPipeline() override;

private:
	VkViewport _viewport;
	VkRect2D   _scissor;
	VkPipelineLayout _pipelineLayout;
	std::vector<Shader> _shaders;
	std::vector<VkDynamicState> _dynamicStates;          // dynamic states
	std::vector<VkFormat> _colorAttachmentFormats;   // real VkFormat storage
	std::vector<VkVertexInputBindingDescription> _inputBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> _inputAttributeDescriptions;
	std::vector<VkPipelineColorBlendAttachmentState> _blendAttachments;
	std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
	std::vector<VkPushConstantRange> _pipelinePushConstantRanges;
	std::vector<VkDescriptorSetLayout> _pipelineDescriptorSetLayouts;
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

struct Image
{
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory memory;
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
		VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME,
		"VK_KHR_pipeline_executable_properties"
	};

	//vulkan
	VulkanContext _vk;
	//VkInstance _instance = nullptr;
	//VkDevice _device = nullptr;
	//VkPhysicalDevice _physicalDevice = nullptr;
	//VkQueue _graphicsQueue, _presentQueue;
	VkSurfaceKHR _surface;
	VkSwapchainKHR _swapchain;
	std::vector<VkImage> _swapchainImages;
	VkFormat _swapchainImageFormat;
	VkExtent2D _swapchainExtent;
	std::vector<VkImageView> _swapchainImageViews;
	std::vector<VkCommandBuffer> _swapchainCommandBuffers;
	std::vector<VkSemaphore> _imageAvailableSemaphores, _renderFinishedSemaphores;
	std::vector<VkFence> _renderingFences;

	bool _frameLocked;
	unsigned _currentFrame, _targetFrame, _maxFrames;



	RETURN(bool) CheckCompatibility(const char** instanceExtensions, const char** deviceExtensions);
	RETURN(bool) CheckLayerSupport();

	RETURN(void) PickPhysicalDevice();
	RETURN(bool) IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice);
	RETURN(QueueFamilyIndices) FindQueueFamilies(VkPhysicalDevice physicalDevice);

	//RETURN(VkImage) CreateImage

	RETURN(void) CreateInstance(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions);
	RETURN(void) CreateDevice();
	RETURN(void) CreateSurface();
	RETURN(void) CreateSwapchain();
	RETURN(void) CreateSwapchainImageViews();
	void CreateGraphicsPipeline();

	RETURN(VkCommandBuffer) StartFrame();
	RETURN(void) EndFrame(VkCommandBuffer commandBuffer);


	RETURN(bool) CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
	RETURN(SwapchainSupportDetails) QuerySwapchainSupport(VkPhysicalDevice physicalDevice);
	RETURN(VkSurfaceFormatKHR) ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	RETURN(VkPresentModeKHR) ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	RETURN(VkExtent2D) ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);


public:
	VulkanBackend()
	{
	}

	~VulkanBackend()
	{
		for (auto imageView : _swapchainImageViews)
		{
			vkDestroyImageView(_vk.device, imageView, nullptr);
		}

		vkDestroyPipeline(_vk.device, _vk.pipeline, nullptr);
		vkDestroyPipelineLayout(_vk.device, _vk.pipelineLayout, nullptr);
		vkDestroySwapchainKHR(_vk.device, _swapchain, nullptr);
		vkDestroySurfaceKHR(_vk.instance, _surface, nullptr);
		vkDestroyInstance(_vk.instance, nullptr);
	}

	RETURN(bool) Init(unsigned layerCount = 0, const char** layers = nullptr, unsigned extensionCount = 0, const char** extensions = nullptr, unsigned dExtensionCount = 0, const char** dExtensions = nullptr);
	//std::expected<bool, const char*> Init(unsigned layerCount, const char** layers, unsigned extensionCount, const char** extensions);
};

