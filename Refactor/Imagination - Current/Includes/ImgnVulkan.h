#include "ImgnWindow.h"
//#include <Windows.h>
#include <wrl/client.h>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
using namespace Microsoft::WRL;

#include "gltf/tiny_gltf.h"
#include "ResourceTypes.h"
#include "RenderGraph.h"

constexpr int MAXFRAMESINFLIGHT = 3;

//struct Vertex
//{
//	Math::vec3<float> pos;
//	Math::vec3<float> nrm;
//	Math::vec2<float> uv0;
//	Math::vec4<float> tan;
//	Math::vec3<float> col;
//
//	static vk::VertexInputBindingDescription GetBindingDescription() { return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex }; }
//	static std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescriptions()
//	{
//		return
//		{
//			vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
//			vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, col)),
//			vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv0)),
//		};
//	}
//};

struct alignas(16) UniformBufferObject
{
	Math::mat4<float> model, view, proj;
};

//struct ModelLoader
//{
//	tinygltf::Model model;
//	std::vector<Vertex> vertices;
//	std::vector<uint32_t> indices;
//
//	vk::Buffer vertexBuffer = nullptr;
//	vk::DeviceMemory vertexBufferMemory = nullptr;
//
//	vk::Buffer indexBuffer = nullptr;
//	vk::DeviceMemory indexBufferMemory = nullptr;
//
//	void LoadModel(const std::string& pFile)
//	{
//		tinygltf::TinyGLTF loader;
//		std::string err;
//		std::string warn;
//
//		bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, pFile);
//		//bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename); // for binary glTF(.glb)
//
//		if (!warn.empty()) std::format("Warn: {}\n", warn);
//
//		if (!err.empty()) std::format("Err: {}\n", err);
//
//		if (!ret) std::format("Failed to parse glTF: {}\n", pFile);
//
//		std::unordered_map<Vertex, uint32_t> uniqueVertices;
//
//		for (auto& mesh : model.meshes)
//		{
//			for (auto& primitive : mesh.primitives)
//			{
//				//indices
//				const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
//				const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
//				const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];
//
//				//pos
//				const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
//				const tinygltf::BufferView& posBufferView = model.bufferViews[posAccessor.bufferView];
//				const tinygltf::Buffer& posBuffer = model.buffers[posBufferView.buffer];
//
//				bool hasNrms = primitive.attributes.find("NORMAL") != primitive.attributes.end();
//				const tinygltf::Accessor* nrmAccessor = nullptr;
//				const tinygltf::BufferView* nrmBufferView = nullptr;
//				const tinygltf::Buffer* nrmBuffer = nullptr;
//
//				if (hasNrms)
//				{
//					nrmAccessor = &model.accessors[primitive.attributes.at("NORMAL")];
//					nrmBufferView = &model.bufferViews[nrmAccessor->bufferView];
//					nrmBuffer = &model.buffers[nrmBufferView->buffer];
//				}
//
//				bool hasTexCoords = primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end();
//				const tinygltf::Accessor* texCoordAccessor = nullptr;
//				const tinygltf::BufferView* texCoordBufferView = nullptr;
//				const tinygltf::Buffer* texCoordBuffer = nullptr;
//
//				if (hasTexCoords)
//				{
//					texCoordAccessor = &model.accessors[primitive.attributes.at("TEXCOORD_0")];
//					texCoordBufferView = &model.bufferViews[texCoordAccessor->bufferView];
//					texCoordBuffer = &model.buffers[texCoordBufferView->buffer];
//				}
//
//				bool hasTans = primitive.attributes.find("TANGENT") != primitive.attributes.end();
//				const tinygltf::Accessor* tanAccessor = nullptr;
//				const tinygltf::BufferView* tanBufferView = nullptr;
//				const tinygltf::Buffer* tanBuffer = nullptr;
//
//				if (hasTans)
//				{
//					tanAccessor = &model.accessors[primitive.attributes.at("TANGENT")];
//					tanBufferView = &model.bufferViews[tanAccessor->bufferView];
//					tanBuffer = &model.buffers[tanBufferView->buffer];
//				}
//
//				for (size_t i = 0; i < posAccessor.count; i++)
//				{
//					Vertex v;
//
//					const float* pos = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset + i * 12]);
//					const float* nrm = hasNrms ? reinterpret_cast<const float*>(&nrmBuffer->data[nrmBufferView->byteOffset + nrmAccessor->byteOffset + i * 12]) : new float[3] { 0.f, 0.f, 0.f };
//					const float* uv0 = hasTexCoords ? reinterpret_cast<const float*>(&texCoordBuffer->data[texCoordBufferView->byteOffset + texCoordAccessor->byteOffset + i * 8]) : new float[2] { 0.f, 0.f };
//					const float* tan = hasTans ? reinterpret_cast<const float*>(&tanBuffer->data[tanBufferView->byteOffset + tanAccessor->byteOffset + i * 16]) : new float[4] { 0.f, 0.f, 0.f, 0.f };
//
//					v.pos = { pos[0], pos[1], pos[2] };
//					v.nrm = { nrm[0], nrm[1], nrm[2] };
//					v.uv0 = { uv0[0], uv0[1] };
//					v.tan = { tan[0], tan[1], tan[2], tan[3] };
//					v.col = { 1.f, 1.f, 1.f };
//
//					if (!uniqueVertices.contains(v))
//					{
//						uniqueVertices[v] = static_cast<uint32_t>(vertices.size());
//						vertices.push_back(v);
//					}
//				}
//
//				// Process indices
//				const unsigned char* indexData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];
//
//				// Handle different index component types
//				if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
//				{
//					const uint16_t* indices16 = reinterpret_cast<const uint16_t*>(indexData);
//					for (size_t i = 0; i < indexAccessor.count; i++)
//					{
//						Vertex vertex = vertices[indices16[i]];
//						indices.push_back(uniqueVertices[vertex]);
//					}
//				}
//				else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
//				{
//					const uint32_t* indices32 = reinterpret_cast<const uint32_t*>(indexData);
//					for (size_t i = 0; i < indexAccessor.count; i++)
//					{
//						Vertex vertex = vertices[indices32[i]];
//						indices.push_back(uniqueVertices[vertex]);
//					}
//				}
//				else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
//				{
//					const uint8_t* indices8 = reinterpret_cast<const uint8_t*>(indexData);
//					for (size_t i = 0; i < indexAccessor.count; i++)
//					{
//						Vertex vertex = vertices[indices8[i]];
//						indices.push_back(uniqueVertices[vertex]);
//					}
//				}
//			}
//		}
//	}
//};

//class Device
//{
//	vk::Device _device;
//	vk::PhysicalDevice _physicalDevice;
//	vk::CommandPool _commandPool;
//	vk::Queue _queue;
//
//	static inline std::unique_ptr<Device> _instance = nullptr;
//	Device() = default;
//
//public:
//
//	static Device& Inst()
//	{
//		if (!_instance) _instance.reset(new Device());
//
//		return *_instance;
//	}
//
//	vk::Device GetDevice() { return _device; }
//	vk::PhysicalDevice GetPhysicalDevice() { return _physicalDevice; }
//
//	void SetDevice(vk::Device pDevice) { _device = pDevice; }
//	void SetPhysicalDevice(vk::PhysicalDevice pPhysicalDevice) { _physicalDevice = pPhysicalDevice; }
//	void SetCommandPool(vk::CommandPool pCommandPool) { _commandPool = pCommandPool; }
//	void SetQueue(vk::Queue pQueue) { _queue = pQueue; }
//
//	vk::CommandBuffer BeginSingleTimeCommand()
//	{
//		vk::CommandBufferAllocateInfo allocInfo
//		{
//			.commandPool = _commandPool,
//			.level = vk::CommandBufferLevel::ePrimary,
//			.commandBufferCount = 1
//		};
//
//		auto commandBuffer = std::move(_device.allocateCommandBuffers(allocInfo).front());
//
//		vk::CommandBufferBeginInfo beginInfo
//		{
//			.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
//		};
//
//		commandBuffer.begin(beginInfo);
//
//		return commandBuffer;
//	}
//
//	void EndSingleTimeCommand(vk::CommandBuffer& pCommandBuffer)
//	{
//		pCommandBuffer.end();
//
//		vk::SubmitInfo submitInfo
//		{
//			.commandBufferCount = 1,
//			.pCommandBuffers = &pCommandBuffer
//		};
//
//		_queue.submit(submitInfo, nullptr);
//		_queue.waitIdle();
//	}
//};

struct Buffer
{
	vk::raii::Buffer buffer = nullptr;
	vk::raii::DeviceMemory memory = nullptr;
};

struct Image
{
	vk::raii::Image image = nullptr;
	vk::raii::ImageView imageView = nullptr;
	vk::raii::DeviceMemory memory = nullptr;
};

static constexpr uint32_t NumDescriptorsStreaming = 2048;

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

	std::unordered_map<std::string, std::wstring> _sTarget =
	{
		std::make_pair("frag", L"ps_6_6"),
		std::make_pair("vert", L"vs_6_6"),
		std::make_pair("comp", L"cs_6_6")
	};

	const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4 };

	const std::vector<Vertex> vertices =
	{
		{{-.5f, -.5f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 0.f}},
		{{.5f, -.5f, 0.f}, {1.f, 1.f, 0.f}, {0.f, 0.f}},
		{{.5f, .5f, 0.f}, {1.f, 0.f, 1.f}, {0.f, 1.f}},
		{{-.5f, .5f, 0.f}, {1.f, 1.f, 1.f}, {1.f, 1.f}},

		{{-.5f, -.5f, -.5f}, {1.f, 0.f, 0.f}, {1.f, 0.f}},
		{{.5f, -.5f, -.5f}, {1.f, 1.f, 0.f}, {0.f, 0.f}},
		{{.5f, .5f, -.5f}, {1.f, 0.f, 1.f}, {0.f, 1.f}},
		{{-.5f, .5f, -.5f}, {1.f, 1.f, 1.f}, {1.f, 1.f}}
	};

	enum class RenderPassIdx : uint32_t
	{
		GBuffer = 0,
		Lighting = 1,
		Count
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
	vk::raii::DescriptorPool _descriptorPool = nullptr;
	std::vector<vk::raii::DescriptorSet> _descriptorSets;
	vk::raii::DescriptorSetLayout _descriptorSetLayout = nullptr;
	uint32_t _totalSets = MAXFRAMESINFLIGHT * 8;

	vk::raii::CommandPool _commandPool = nullptr;
	std::vector<vk::raii::CommandBuffer> _commandBuffers;

	std::vector<vk::raii::Fence> _inFlightFences;
	std::vector<vk::raii::Semaphore> _renderFinishedSemaphores;
	std::vector<vk::raii::Semaphore> _presentCompleteSemaphores;

	Buffer _indexBuffer;
	Buffer _vertexBuffer;

	std::vector<Buffer> _uniformBuffers;
	std::vector<void*> _uniformsBuffersMapped;

	Image _depth;
	Image _texture;
	std::optional<vk::raii::Sampler> _textureSampler;

	ResourceHandle<Mesh> _sponza;

	static constexpr uint32_t DescriptorSetIndex(uint32_t pFrameIdx, RenderPassIdx pPass)
	{
		return pFrameIdx * static_cast<uint32_t>(RenderPassIdx::Count) + static_cast<uint32_t>(pPass);
	}

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
	void TransitionImageLayout(const vk::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::AccessFlags2 pSrcAccessMask, vk::AccessFlags2 pDstAccessMask, vk::PipelineStageFlags2 pSrcStageMask, vk::PipelineStageFlags2 pDstStageMask, vk::ImageAspectFlags pImageAspectFlags);
	void TransitionImageLayout(const vk::raii::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::AccessFlags2 pSrcAccessMask, vk::AccessFlags2 pDstAccessMask, vk::PipelineStageFlags2 pSrcStageMask, vk::PipelineStageFlags2 pDstStageMask, vk::ImageAspectFlags pImageAspectFlags);

	vk::raii::CommandBuffer BeginSingleCommand();
	void EndSingleCommand(vk::raii::CommandBuffer& pCommandBuffer);

	void UpdateUniformBuffer(uint32_t pCurrImage);
	vk::raii::ImageView CreateImageView(vk::Image& pImage, vk::Format pFormat, vk::ImageAspectFlags pAspectFlags);
	vk::raii::ImageView CreateImageView(vk::raii::Image& pImage, vk::Format pFormat, vk::ImageAspectFlags pAspectFlags);
	void CopyBuffer(vk::raii::Buffer& pSrc, vk::raii::Buffer& pDst, vk::DeviceSize pSize);
	void CopyBufferToImage(const vk::raii::Buffer& pBuffer, vk::raii::Image& pImage, uint32_t pWidth, uint32_t pHeight);
	void CreateBuffer(vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Buffer& pBuffer);
	void CreateImage(uint32_t pWidth, uint32_t pHeight, vk::Format pFormat, vk::ImageTiling pTiling, vk::ImageUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Image& pImage);
	void UpdateDescriptorSet(uint32_t pFrameIdx, RenderPassIdx pIdx, vk::Buffer pUniformBuffer, vk::DeviceSize pUniformBufferSize, vk::Buffer pStorageBuffer, vk::DeviceSize pStorageBufferSize, const std::vector<vk::ImageView>& pTextures, vk::Sampler pSampler);

	void CreateDevice();
	void CreateSurface();
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
	void CreateTextureSampler();
	void CreateGraphicsPipeline();
	void CreateTextureImageView();
	void CreateDescriptorSetLayout();
	
	RenderGraph _graph;
	ResourceManager _manager;

	std::array<Image, 3> _gBufferImages;
	Image _finalImage;

	void SetupDeferredRenderer();

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