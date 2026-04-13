#pragma once

/*add passes
if resource output add format to pass colorattachment*/

//constexpr const wchar_t* VertexTarget = L"vs_6_6";
//constexpr const wchar_t* FragmentTarget = L"ps_6_6";
//constexpr const wchar_t* ComputeTarget = L"cs_6_6";

//#define V_SHADER(x) Shader::##x##VertexShader
//#define F_SHADER(x) Shader::##x##FragmentShader
//#define C_SHADER(x) Shader::##x##ComputeShader

struct Pipelines
{
	vk::raii::Pipeline gBufferPipeline = nullptr, lightingPipeline = nullptr;
	vk::raii::PipelineLayout pipelineLayout = nullptr;
};

struct RenderPass
{
	std::string name;
	std::vector<std::string> inputs;
	std::vector<std::string> outputs;

	vk::Pipeline pipeline;
	vk::PipelineLayout pipelineLayout;
	vk::DescriptorSetLayout descriptorSetLayout;
	std::vector<vk::Format> colorAttachments;

	std::function<void(vk::raii::CommandBuffer&)> Execute;
};

class RenderGraph
{
	struct ImageResource
	{
		std::string name;
		vk::Format format;
		vk::Extent2D extent;
		vk::ImageUsageFlags usage;
		vk::ImageLayout initLayout;
		vk::ImageLayout finalLayout;
		vk::ImageAspectFlags aspect;

		// Actual GPU resources - populated during compilation
		vk::raii::Image image = nullptr;      // The GPU image object
		vk::raii::DeviceMemory memory = nullptr;  // Backing memory allocation
		vk::raii::ImageView view = nullptr;   // Shader-accessible view of the image
	};

	struct BufferResource
	{
		std::string name;
		uint64_t size;
		vk::BufferUsageFlags usage;

		vk::raii::Buffer buffer = nullptr;
		vk::raii::DeviceMemory memory = nullptr;
	};

	struct Pass
	{
		std::string name;
		std::vector<std::string> inputs;
		std::vector<std::string> outputs;

		//vk::raii::Pipeline pipeline;
		//vk::raii::PipelineLayout pipelineLayout;

		std::function<void(vk::raii::CommandBuffer&)> Execute;
	};

	std::unordered_map<std::string, ImageResource> _resources;
	std::vector<Pass> _passes;
	std::vector<RenderPass> _renderPasses;
	std::vector<uint64_t> _executionOrder;

	std::vector<vk::raii::Semaphore> _semaphores;
	std::vector<std::pair<uint64_t, uint64_t>> _semaphoreSignalWaitPairs;

	uint32_t FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps);
	void TransitionImageLayout(vk::raii::CommandBuffer& pCommandBuffer, const vk::raii::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::AccessFlags2 pSrcAccessMask, vk::AccessFlags2 pDstAccessMask, vk::PipelineStageFlags2 pSrcStageMask, vk::PipelineStageFlags2 pDstStageMask, vk::ImageAspectFlags pImageAspectFlags);

public:
	explicit RenderGraph() = default;

	void Compile();
	void Execute(vk::raii::CommandBuffer& pCommandBuffer, vk::Queue pQueue);

	// Comprehensive frame rendering with proper synchronization
	// This function demonstrates the complete cycle of frame rendering coordination
	void RenderFrame(vk::Queue pGraphicsQueue, vk::Queue pPresentQueue);


	// Resource access interface for retrieving compiled resources
	ImageResource* GetResource(const std::string& pName)
	{
		auto it = _resources.find(pName);
		return (it != _resources.end()) ? &it->second : nullptr;
	}

	void AddResource(const std::string& pName, vk::Format pFormat, vk::Extent2D pExtent, vk::ImageUsageFlags pUsage, vk::ImageLayout pInitialLayout, vk::ImageLayout pFinalLayout, vk::ImageAspectFlags pAspect);
	void AddPass(const std::string& pName, const std::vector<std::string>& pInputs, const std::vector<std::string>& pOutputs, std::function<void(vk::raii::CommandBuffer&)> pExecute);
	void AddPass(RenderPass& pRenderPass) { _renderPasses.emplace_back(std::move(pRenderPass)); }
};

