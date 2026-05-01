#pragma once
#include <wrl/client.h>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
using namespace Microsoft::WRL;

#include "RenderGraph.h"

namespace ImgnVulkan
{
	class IMGN_VULKAN_API Ctx
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
		};

		enum class RenderPassIdx : uint32_t
		{
			GBuffer = 0,
			Lighting = 1,
			Count
		};

		static constexpr uint32_t DescriptorSetIndex(uint32_t pFrameIdx, RenderPassIdx pPass)
		{
			return pFrameIdx * static_cast<uint32_t>(RenderPassIdx::Count) + static_cast<uint32_t>(pPass);
		}

		//Pipelines _pipelines;

		RenderGraph _graph;

		uint32_t _width, _height;

		bool IsDeviceSuitable(vk::raii::PhysicalDevice const& pPhysicalDevice);
		void CleanupSwapchain();
		void RecreateSwapchain();
		void PickPhysicalDevice();
		void SetupDebugMessenger();
		vk::Format FindDepthFormat();
		void RecordCommandBuffer(uint32_t pImageIdx);
		bool HasStencilComponent(vk::Format pFormat);
		uint32_t FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps);
		vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const& pCapabilities);
		uint32_t ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& pCapabilities);
		[[nodiscard]] vk::raii::ShaderModule CreateShaderModule(const std::vector<uint32_t>& pCode) const;
		vk::PresentModeKHR ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& pAvailablePresentModes);
		vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& pAvailableFormats);
		std::vector<uint32_t> GetSPV(const std::string& pShader, const std::wstring& pTarget, const std::wstring& pEntryPoint = L"main");
		vk::Format FindSupportedFormat(const std::vector<vk::Format>& pCandidates, vk::ImageTiling pTiling, vk::FormatFeatureFlags pFeatures);
		void TransitionImageLayout(const vk::raii::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout);
		void TransitionImageLayout(const vk::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout);
		void TransitionImageLayout(vk::raii::CommandBuffer& pCommandBuffer, const vk::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout);
		void TransitionImageLayout(const vk::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::AccessFlags2 pSrcAccessMask, vk::AccessFlags2 pDstAccessMask, vk::PipelineStageFlags2 pSrcStageMask, vk::PipelineStageFlags2 pDstStageMask, vk::ImageAspectFlags pImageAspectFlags);
		void TransitionImageLayout(const vk::raii::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::AccessFlags2 pSrcAccessMask, vk::AccessFlags2 pDstAccessMask, vk::PipelineStageFlags2 pSrcStageMask, vk::PipelineStageFlags2 pDstStageMask, vk::ImageAspectFlags pImageAspectFlags);

		vk::raii::CommandBuffer BeginSingleCommand();
		void EndSingleCommand(vk::raii::CommandBuffer& pCommandBuffer);

		vk::raii::ImageView CreateImageView(vk::Image& pImage, vk::Format pFormat, vk::ImageAspectFlags pAspectFlags);
		vk::raii::ImageView CreateImageView(vk::raii::Image& pImage, vk::Format pFormat, vk::ImageAspectFlags pAspectFlags);
		void CopyBuffer(vk::raii::Buffer& pSrc, vk::raii::Buffer& pDst, vk::DeviceSize pSize);
		void CopyBufferToImage(const vk::raii::Buffer& pBuffer, vk::raii::Image& pImage, uint32_t pWidth, uint32_t pHeight);
		void CreateBuffer(vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Buffer& pBuffer);
		void CreateImage(uint32_t pWidth, uint32_t pHeight, vk::Format pFormat, vk::ImageTiling pTiling, vk::ImageUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Image& pImage);

		void UpdateDescriptorSet(uint32_t pFrameIdx, RenderPassIdx pIdx, vk::Buffer pUniformBuffer, vk::DeviceSize pUniformBufferSize, vk::Buffer pStorageBuffer, vk::DeviceSize pStorageBufferSize, const std::vector<vk::ImageView>& pTextures, vk::Sampler pSampler);

		void CreateDevice();
		void CreateSurface(HWND pHWND);
		void CreateInstance();
		void CreateSwapchain();
		void CreateImageViews();
		void CreateCommandPool();
		void CreateSyncObjects();
		void CreateIndexBuffer();
		void CreateVertexBuffer();
		void CreateTextureImage();
		void CreateCommandBuffer();
		void CreateDepthResources();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateDefaultSampler();
		void CreateGraphicsPipelines();
		void CreateTextureImageView();
		void CreateDescriptorSetLayout();

		void SetupDeferredRenderer();


	public:
		/* Class Defaults */
		Ctx()
		{

		}

		~Ctx()
		{

		}

		/* Class Functions */
		void InitVulkanCtx(HWND pHWND, uint32_t pWidth, uint32_t pHeight);
		void UploadMesh(const Vertex* pVertices, uint64_t pVertexCount);
		void UploadIndices(const uint32_t* pIndices, uint64_t pIndexCount);
		void CreateImage(const std::string& pName, vk::Format pFormat, vk::Extent2D pExtent, vk::ImageUsageFlags pUsage, vk::ImageLayout pInitialLayout, vk::ImageLayout pFinalLayout, vk::ImageAspectFlags pAspect);
		void CreateBuffer(const std::string& pName, vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, const void* pData);
		void CompileGraph();
		void DrawFrame();
	};
}