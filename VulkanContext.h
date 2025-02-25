#pragma once
using namespace Microsoft::WRL;

namespace VkContext
{
	enum VertexInput
	{
		POSITION = 1 << 0,
		NORMAL = 1 << 1,
		TEXCOORD = 1 << 2,
		TANGENT = 1 << 3
	};

	enum Topology : unsigned char
	{
		POINT_TOPOLOGY,
		LINE_TOPOLOGY,
		LINE_STRIP_TOPOLOGY,
		TRIANGLE_TOPOLOGY,
		TRIANGLE_STRIP_TOPOLOGY
	};

	enum PolygonMode : unsigned char
	{
		FILL,
		LINE,
	};

	enum CullMode : unsigned char
	{
		FRONT,
		BACK,
		NONE
	};

	enum FrontFace : unsigned char
	{
		CLOCKWISE,
		COUNTER_CLOCKWISE
	};



	struct PipelineDescription
	{
		Topology topology = TRIANGLE_TOPOLOGY;
		VertexInput vertexInput;

		std::unique_ptr<Shader> vertexShader;
		std::unique_ptr<Shader> fragmentShader;
		//... rest of shader types


		VkFormat depthFormat;

		PolygonMode polygonMode = FILL;
		CullMode cullMode = NONE;
		FrontFace frontFace = COUNTER_CLOCKWISE;
	};

	class VulkanContext
	{
		static inline VulkanContext* _vulkanContext = nullptr;
		GVulkanSurface _vulkanSurface;

		//vulkan
		VkDevice _device;
		VkPhysicalDevice _physicalDevice;
		VkInstance _instance;
		VkCommandPool _commandPool;
		VkQueue _graphicsQueue;
		VkSwapchainKHR _swapchain;
		VkRenderPass _renderPass;
		VkFramebuffer _frameBuffer;

		unsigned int _maxFramesInFlight = 0;
		float _aspectRatio;

		//dxc
		ComPtr<IDxcCompiler3> _compiler;
		ComPtr<IDxcUtils> _utils;
		ComPtr<IDxcIncludeHandler> _includeHandler;

		VulkanContext()
		{

		}

		VulkanContext(GWindow win)
		{
#ifndef NDEBUG
			std::vector<const char*> debugLayers =
			{
				"VK_LAYER_KHRONOS_validation"
			};

			std::vector<const char*> deviceExt =
			{
				"VK_KHR_dynamic_rendering"
			};

			if (+_vulkanSurface.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT | GW::GRAPHICS::TRIPLE_BUFFER, debugLayers.size(), debugLayers.data(), 0, nullptr, deviceExt.size(), deviceExt.data(), false))
#else
			if (+_vulkanSurface.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT | GW::GRAPHICS::TRIPLE_BUFFER))
#endif
			{
				_vulkanSurface.GetDevice((void**)&_device);
				_vulkanSurface.GetPhysicalDevice((void**)&_physicalDevice);
				_vulkanSurface.GetInstance((void**)&_instance);
				_vulkanSurface.GetGraphicsQueue((void**)&_graphicsQueue);
				_vulkanSurface.GetCommandPool((void**)&_commandPool);
				_vulkanSurface.GetGraphicsQueue((void**)&_graphicsQueue);
				_vulkanSurface.GetSwapchain((void**)&_swapchain);

				//dxc
				DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
				DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
				_utils->CreateDefaultIncludeHandler(&_includeHandler);
				std::filesystem::create_directories("Shaders/SPV");
			}
		}

		~VulkanContext()
		{

		}

	public:

		static VulkanContext* GetInst()
		{
			if (!_vulkanContext) _vulkanContext = new VulkanContext();

			return _vulkanContext;
		}

		static VulkanContext* GetInst(GWindow win)
		{
			if (!_vulkanContext) _vulkanContext = new VulkanContext(win);

			return _vulkanContext;
		}

		VkDevice GetDevice() const { return _device; }
		VkPhysicalDevice GetPhysicalDevice() const { return _physicalDevice; }
		VkInstance GetInstance() const { return _instance; }
		VkCommandPool GetCommandPool() const { return _commandPool; }
		VkQueue GetGraphicsQueue() const { return _graphicsQueue; }
		VkSwapchainKHR GetSwapchain() const { return _swapchain; }
		VkRenderPass GetRenderPass() const { return _renderPass; }
		VkFramebuffer GetFrameBuffer(int idx);

		ComPtr<IDxcCompiler3> GetCompiler() const { return _compiler; }
		ComPtr<IDxcUtils> GetUtils() const { return _utils; }
		ComPtr<IDxcIncludeHandler> GetIncludeHandler() const { return _includeHandler; }

		VkPipeline CreateGraphicsPipeline(PipelineDescription pipelineDescription);
	};
}