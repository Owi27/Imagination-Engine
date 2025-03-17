#pragma once
class Texture;

class RenderPass
{
	std::shared_ptr<VulkanContext> _vk;
	FrameGraph& _graph;
	std::string _name;
	FrameGraphQueueBit _queue;
	std::vector<std::shared_ptr<Texture>> _colorInputs;
	std::vector<std::shared_ptr<Texture>> _colorOutputs;
	std::vector<std::shared_ptr<Buffer>> _bufferInputs;
	std::vector<std::shared_ptr<Buffer>> _bufferOutputs;
	std::shared_ptr<Texture> _depthStencilInput = nullptr;
	std::shared_ptr<Texture> _depthStencilOutput = nullptr;

	VkCommandBuffer _commandBuffer;
	VkPipelineLayout _pipelineLayout;
	VkPipeline _pipeline;
	VkSemaphore _semaphore;
	PipelineDescription _pipelineDescription;

	std::function<void(VkCommandBuffer&)> _drawCalls;

	VkDescriptorSet _descriptorSet;

public:
	RenderPass(VulkanContext& vk, FrameGraph& graph, FrameGraphQueueBit queue) : _graph(graph)
	{
		_vk = std::make_shared<VulkanContext>(vk);
		_queue = queue;
	}

	~RenderPass()
	{

	}

	Texture& AddTextureInput(std::string name);
	Texture& AddTextureOutput(const std::string& name, const VkFormat format, const std::string& input = "");
	Texture& AddDepthOutput(const std::string& name);
	Buffer& AddBufferInput(std::string name);
	Buffer& AddBufferOutput(const std::string& name, unsigned size, void* data, const VkBufferUsageFlags usageFlags, const std::string& input = "");

	// should have color attachments
	void Setup();
	void Execute();

	VkCommandBuffer& GetCommandBuffer() { return _commandBuffer; }
	VkPipeline& GetPipeline() { return _pipeline; }
	VkPipelineLayout& GetPipelineLayout() { return _pipelineLayout; }
	VkDescriptorSet& GetDescriptorSet() { return _descriptorSet; }
	VkSemaphore& GetSemaphore() { return _semaphore; }

	std::vector<std::shared_ptr<Buffer>>& GetBufferOutputs() { return _bufferOutputs; }

	void CreatePipeline(const PipelineDescription pipelineDescription);
	void SetPipeline(VkPipeline& pipeline) { _pipeline = pipeline; }
	void SetDrawCalls(std::function<void(VkCommandBuffer&)> drawCalls) { _drawCalls = std::move(drawCalls); }
};

