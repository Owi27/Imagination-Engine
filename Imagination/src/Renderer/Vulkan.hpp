#pragma once
#include <wrl/client.h>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
using namespace Microsoft::WRL;

constexpr int MaxFramesInFlight = 2;
constexpr const wchar_t* VertexTarget = L"vs_6_6";
constexpr const wchar_t* FragmentTarget = L"ps_6_6";
constexpr const wchar_t* ComputeTarget = L"cs_6_6";
constexpr uint32_t NumDescriptorsStreaming = 128;

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

struct RGImage //literally just image, but with rendergraph info
{
	Image image;

	vk::AccessFlags2 currentAccess = vk::AccessFlagBits2::eNone;
	vk::ImageLayout currentLayout = vk::ImageLayout::eUndefined;
	vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;
	vk::PipelineStageFlags2 currentStage = vk::PipelineStageFlagBits2::eNone;
};

struct RGBuffer
{
	Buffer buffer;

	vk::AccessFlags2 currentAccess = vk::AccessFlagBits2::eNone;
	vk::PipelineStageFlags2 currentStage = vk::PipelineStageFlagBits2::eNone;
};

class Vulkan
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

	RendererCreateInfo _info;

	unique<vk::raii::Context> _ctx = Unique<vk::raii::Context>();
	unique<vk::raii::Instance> _instance;
	unique<vk::raii::Device> _device;
	unique<vk::raii::PhysicalDevice> _physicalDevice;
	unique<vk::raii::CommandPool> _commandPool;
	unique<vk::raii::Queue> _queue;
	unique<vk::raii::DebugUtilsMessengerEXT> _debugMessenger;
	unique<vk::raii::SurfaceKHR> _surface;

	//std::vector<unique<vk::raii::CommandBuffer>> _commandBuffers;

	/*Swapchain*/
	vk::Extent2D _swapchainExtent;
	unique<vk::raii::SwapchainKHR> _swapchain;
	vk::SurfaceFormatKHR _swapchainSurfaceFormat;
	std::vector<vk::Image> _swapchainImages;
	std::vector<unique<vk::raii::ImageView>> _swapchainImageViews;

	/*Descriptors*/
	unique<vk::raii::DescriptorPool> _textureDescriptorPool;
	unique<vk::raii::DescriptorSet> _textureDescriptorSet;
	unique<vk::raii::DescriptorSetLayout> _pushDescriptorSetLayout, _textureDescriptorSetLayout;

	unique<vk::raii::Sampler> _textureSampler;

	uint32_t _queueIdx = 0;// , _frameIdx = 0;

	//void CleanupSwapchain();
	//void RecreateSwapchain();
	void RecreateSwapchain();
	void PickPhysicalDevice();
	void SetupDebugMessenger();
	void RecreateSurfaceAndSwapchain();

	uint32_t FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps);
	vk::Extent2D ChooseSwapchainExtent(vk::SurfaceCapabilitiesKHR const& pCapabilities);
	void CopyBuffer(vk::raii::Buffer& pSrc, vk::raii::Buffer& pDst, vk::DeviceSize pSize);
	uint32_t ChooseSwapchainMinImageCount(vk::SurfaceCapabilitiesKHR const& pCapabilities);
	vk::PresentModeKHR ChooseSwapchainPresentMode(std::vector<vk::PresentModeKHR> const& pAvailablePresentModes);
	vk::SurfaceFormatKHR ChooseSwapchainSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& pAvailableFormats);
	void CopyBufferToImage(uint32_t pWidth, uint32_t pHeight, const vk::raii::Buffer& pBuffer, vk::raii::Image& pImage);

	void CreateDXC();
	void CreateDevice();
	void CreateSurface(void* pWindowHandle);
	void CreateInstance();
	void CreateSwapchain();
	void CreateSwapchainImageViews();
	void CreateCommandPool();
	void CreateSyncObjects();
	//void CreateIndexBuffer();
	//void CreateVertexBuffer();
	void CreateCommandBuffers();
	//void CreateDepthResources();
	//void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSets();
	void CreateTextureSampler();
	//void CreateTextureImageView();
	void CreateGraphicsPipelines();
	void CreateDescriptorSetLayout();

	void CreateImageView(vk::Format pFormat, vk::ImageAspectFlags pAspectFlags, Image& pImage);
	void CreateBuffer(vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Buffer& pBuffer);
	void CreateImage(uint32_t pWidth, uint32_t pHeight, vk::Format pFormat, vk::ImageTiling pTiling, vk::ImageUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Image& pImage);

	uint32_t _activeImageIdx = 0, _frameInFlightIdx = 0;
	std::array<unique<vk::raii::Fence>, MaxFramesInFlight> _frameFinishedFence;
	std::array<unique<vk::raii::CommandBuffer>, MaxFramesInFlight> _commandBuffers;
	std::array<unique<vk::raii::Semaphore>, MaxFramesInFlight> _imageAcquiredSemaphores;
	std::vector<unique<vk::raii::Semaphore>> _presentationReadySemaphore;

public:
	Pipelines _pipelines;


	/* Class Defaults */
	Vulkan()
	{

	}

	~Vulkan()
	{

	}

	/* Class Functions */
	void Init(RendererCreateInfo pCreateInfo);
	
	bool StartFrame();
	void EndFrame();
	void EndFrame(RGImage& pImage);

	Buffer CreateVertexBuffer(void* pData, uint64_t pSize);
	Buffer CreateIndexBuffer(void* pData, uint64_t pSize);
	Buffer CreateUniformBuffer(void* pData, uint64_t pSize);
	Buffer CreateStorageBuffer(void* pData, uint64_t pSize);

	RGBuffer CreateRenderBuffer(void* pData, uint64_t pSize, vk::BufferUsageFlags pUsage);

	void MapBufferData(void* pData, uint64_t pSize, Buffer* pBuffer);

	vk::raii::Semaphore CreateVkSemaphore();
	//template<typename T>
	//void CreateUniformBuffer(void* pData, uint64_t pSize = sizeof(T));

	//template<typename T>
	//void CreateStorageBuffer(void* pData, uint64_t pSize = sizeof(T));

	Image CreateDepthImage(uint32_t pWidth, uint32_t pHeight);
	Image CreateTextureImage(uint32_t pWidth, uint32_t pHeight, const uint8_t* pData);
	Image CreateTextureImage(const std::string& pFile);
	RGImage CreateRenderImage(uint32_t pWidth, uint32_t pHeight, vk::Format pFormat, vk::ImageAspectFlags pAspect);

	vk::raii::DescriptorSet& GetTextureDescriptorSet() const { return *_textureDescriptorSet; }
	void UpdateImageDescriptor(uint32_t pSlot, const Image& pImage);

	// For general transitions inside an existing command buffer
	void TransitionImageLayout(vk::CommandBuffer pCommandBuffer, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::Image pImage, vk::ImageAspectFlags pAspect = vk::ImageAspectFlagBits::eColor);

	// For one-off transitions (like texture loading)
	void TransitionImageLayout(vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::Image pImage, vk::ImageAspectFlags pAspect = vk::ImageAspectFlagBits::eColor);
	void TransitionBuffer(vk::PipelineStageFlags2 pNewStage, vk::AccessFlags2 pNewAccess, Buffer& pBuffer);

	vk::raii::CommandBuffer& GetCurrentCommandBuffer() { return *_commandBuffers[_frameInFlightIdx]; }

	unique<vk::raii::CommandBuffer> StartSingleTimeCommand();
	void EndSingleTimeCommand(vk::raii::CommandBuffer& pCommandBuffer);
	
};