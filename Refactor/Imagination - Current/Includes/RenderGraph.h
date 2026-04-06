#pragma once
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

	struct Pass
	{
		std::string name;
		std::vector<std::string> inputs;
		std::vector<std::string> outputs;
		std::function<void(vk::raii::CommandBuffer&)> Execute;
	};

	std::unordered_map<std::string, ImageResource> _resources;
	std::vector<Pass> _passes;
	std::vector<uint64_t> _executionOrder;

	std::vector<vk::raii::Semaphore> _semaphores;
	std::vector<std::pair<uint64_t, uint64_t>> _semaphoreSignalWaitPairs;

	uint32_t FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps);

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
};

