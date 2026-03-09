#define VK_USE_PLATFORM_WIN32_KHR
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#include "ImgnWindow.h"
#include "ImgnMath.h"
//#include <Windows.h>
#include <wrl/client.h>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
using namespace Microsoft::WRL;
using namespace Math;

constexpr int MAXFRAMESINFLIGHT = 3;

struct Vertex
{
	vec2<float> pos;
	vec3<float> col;

	static vk::VertexInputBindingDescription GetBindingDescription() { return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex }; }
	static std::array<vk::VertexInputAttributeDescription, 2> GetAttributeDescriptions()
	{
		return
		{
			vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)),
			vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, col)),
		};
	}
};

struct vkBuffer
{
	vk::raii::Buffer buffer = nullptr;
	vk::raii::DeviceMemory memory = nullptr;
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
		vk::KHRSwapchainExtensionName
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

	std::unordered_map<std::string, std::wstring> _sTarget =
	{
		std::make_pair("frag", L"ps_6_6"),
		std::make_pair("vert", L"vs_6_6"),
		std::make_pair("comp", L"cs_6_6")
	};

	const std::vector<Vertex> vertices =
	{
		{{0.f, -.5f,}, {1.f, 0.f, 0.f}},
		{{.5f, .5f,}, {1.f, 1.f, 0.f}},
		{{-.5f, .5f,}, {1.f, 0.f, 1.f}},
	};

	ImgnWindow* _win;

	//dxc
	ComPtr<IDxcCompiler3> _compiler;
	ComPtr<IDxcUtils> _utils;
	ComPtr<IDxcIncludeHandler> _includeHandler;

	vk::raii::Context _ctx;
	vk::Extent2D _swapchainExtent;
	bool _framebufferResized = false;
	vk::raii::Device _device = nullptr;
	std::optional<vk::raii::Queue> _queue;
	uint32_t _queueIdx = 0, _frameIdx = 0;
	vk::raii::Instance _instance = nullptr;
	vk::raii::SurfaceKHR _surface = nullptr;
	vk::raii::SwapchainKHR _swapchain = nullptr;
	vk::SurfaceFormatKHR _swapchainSurfaceFormat;
	vk::raii::PhysicalDevice _physicalDevice = nullptr;
	std::vector<vk::Image> _swapchainImages;
	std::vector<vk::raii::ImageView> _swapchainImageViews;
	vk::raii::DebugUtilsMessengerEXT _debugMessenger = nullptr;

	vk::raii::Pipeline _pipeline = nullptr;
	vk::raii::PipelineLayout _pipelineLayout = nullptr;

	vk::raii::CommandPool _commandPool = nullptr;
	std::vector<vk::raii::CommandBuffer> _commandBuffers;

	std::vector<vk::raii::Semaphore> _presentCompleteSemaphores;
	std::vector<vk::raii::Semaphore> _renderFinishedSemaphores;
	std::vector<vk::raii::Fence> _inFlightFences;

	vkBuffer _vertexBuffer;

	bool IsDeviceSuitable(vk::raii::PhysicalDevice const& pPhysicalDevice);
	void CleanupSwapchain();
	void RecreateSwapchain();
	void PickPhysicalDevice();
	void SetupDebugMessenger();
	void RecordCommandBuffer(uint32_t pImageIdx);
	uint32_t FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps);
	vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const& pCapabilities);
	uint32_t ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& pCapabilities);
	vk::PresentModeKHR ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& pAvailablePresentModes);
	vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& pAvailableFormats);
	std::vector<uint32_t> GetSPV(const std::string& pShader, const std::wstring& pTarget, const std::wstring& pEntryPoint = L"main");
	void TransitionImageLayout(uint32_t pImageIdx, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::AccessFlags2 pSrcAccessMask, vk::AccessFlags2 pDstAccessMask, vk::PipelineStageFlags2 pSrcStageMask, vk::PipelineStageFlags2 pDstStageMask);
	[[nodiscard]] vk::raii::ShaderModule CreateShaderModule(const std::vector<uint32_t>& pCode) const;

	void CopyBuffer(vk::raii::Buffer& pSrc, vk::raii::Buffer& pDst, vk::DeviceSize pSize);
	void CreateBuffer(vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, vk::MemoryPropertyFlags pProps, vkBuffer& pBuffer);

	void CreateDevice();
	void CreateSurface();
	void CreateInstance();
	void CreateSwapchain();
	void CreateImageViews();
	void CreateCommandPool();
	void CreateSyncObjects();
	void CreateVertexBuffer();
	void CreateCommandBuffer();
	void CreateGraphicsPipeline();


public:
	ImgnVulkan() /*Constructor*/
	{
	}

	~ImgnVulkan() /*Destructor*/
	{
	}

	void Cleanup();
	void InitVulkan(ImgnWindow* pWindow);
	void DrawFrame();
	void DeviceWaitIdle() { _device.waitIdle(); }

	/*Copy Constructor*/
	ImgnVulkan(const ImgnVulkan& pOther) = delete;

	/*Copy Assignment Operator*/
	ImgnVulkan& operator=(const ImgnVulkan& pOther) = delete;

	/*Move Constructor*/
	ImgnVulkan(ImgnVulkan&& pOther) noexcept = default;

	/*Move Assignment Operator*/
	ImgnVulkan& operator=(ImgnVulkan&& pOther) noexcept = default;
};