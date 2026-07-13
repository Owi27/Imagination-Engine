#pragma once
#include <wrl/client.h>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
using namespace Microsoft::WRL;

constexpr const wchar_t* VertexTarget = L"vs_6_6";
constexpr const wchar_t* FragmentTarget = L"ps_6_6";
constexpr const wchar_t* ComputeTarget = L"cs_6_6";
constexpr uint32_t NumDescriptorsStreaming = 2048;

struct Pipelines
{
	unique<vk::raii::Pipeline> gBufferPipeline, shadowPipeline, lightingPipeline;
	unique<vk::raii::PipelineLayout> pipelineLayout;
};

struct Buffer
{
	unique<vk::raii::Buffer> buffer;
	unique<vk::raii::DeviceMemory> memory;
};

struct Image
{
	unique<vk::raii::Image> image;
	unique<vk::raii::ImageView> view;
	unique<vk::raii::DeviceMemory> memory;
};

class ImgnVulkan
{
#ifdef NDEBUG
	const bool _enableValidationLayers = false;
#else
	const bool _enableValidationLayers = true;
#endif

	const std::vector<const char*> _instanceLayers =
	{
#ifdef NDEBUG
#else
		"VK_LAYER_KHRONOS_validation",
#endif
	};

	std::vector<const char*> _instanceExtensions =
	{
		"VK_KHR_surface",
		"VK_KHR_win32_surface",
		vk::EXTDebugUtilsExtensionName
	};

	std::vector<const char*> _deviceExtensions =
	{
		vk::KHRSwapchainExtensionName,
		vk::EXTDescriptorIndexingExtensionName
		//VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		//VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
		//VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME,
		//"VK_KHR_pipeline_executable_properties",
		//VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
		//VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		//VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
		//VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
		//VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
		//VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME
	};

	//dxc
	ComPtr<IDxcCompiler3> _compiler;
	ComPtr<IDxcUtils> _utils;
	ComPtr<IDxcIncludeHandler> _includeHandler;

	unique<vk::raii::Context> _ctx;
	unique<vk::raii::Instance> _instance;
	unique<vk::raii::Device> _device;
	unique<vk::raii::PhysicalDevice> _physicalDevice;
	unique<vk::raii::CommandPool> _commandPool;
	unique<vk::raii::Queue> _queue;
	unique<vk::raii::DebugUtilsMessengerEXT> _debugMessenger;
	unique<vk::raii::SurfaceKHR> _surface;

	/*Swapchain*/
	unique<vk::raii::SwapchainKHR> _swapchain;
	std::vector<vk::raii::Image> _swapchainImages;
	std::vector<vk::raii::ImageView> _swapchainImageViews;

	/*Descriptors*/
	unique<vk::raii::DescriptorPool> _descriptorPool;
	unique<vk::raii::DescriptorSetLayout> _descriptorSetLayout;

	Pipelines _pipelines;

	uint32_t _queueIdx = 0, _frameIdx = 0;

	void CleanupSwapchain();
	void RecreateSwapchain();
	void PickPhysicalDevice();
	void SetupDebugMessenger();

	void CreateDevice();
	void CreateSurface(void* pWindowHandle);
	void CreateInstance();
	void CreateSwapchain();
	void CreateSwapchainImageViews();
	void CreateCommandPool();
	void CreateSyncObjects();
	void CreateIndexBuffer();
	void CreateVertexBuffer();
	void CreateCommandBuffer();
	void CreateDepthResources();
	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSets();
	void CreateTextureSampler();
	void CreateTextureImageView();
	void CreateGraphicsPipelines();
	void CreateDescriptorSetLayout();
	void CreateImageView(vk::Format pFormat, vk::ImageAspectFlags pAspectFlags, Image& pImage);

public:
	/* Class Defaults */
	ImgnVulkan()
	{

	}

	~ImgnVulkan()
	{

	}

	/* Class Functions */
	void Init(RendererCreateInfo pCreateInfo);

	void CreateTextureImage(uint32_t pWidth, uint32_t pHeight, const uint8_t* pData);
	void CreateTextureImage(const std::string& pFile);
	
};