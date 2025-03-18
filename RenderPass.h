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

	VkDescriptorPool _descriptorPool;
	VkDescriptorSetLayout _descriptorSetLayout;
	VkDescriptorSet _descriptorSet;

	std::vector<VkDescriptorPoolSize> _descriptorPoolSizes;
	std::vector<VkDescriptorSetLayoutBinding> _descriptorSetLayoutBindings;

	std::vector<Renderable> _renderables;
	VkPushConstantRange _pushConstantRange;
	bool _usingPushConstant = false;

public:
	RenderPass(VulkanContext& vk, FrameGraph& graph, FrameGraphQueueBit queue) : _graph(graph)
	{
		_vk = std::make_shared<VulkanContext>(vk);
		_queue = queue;

		VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		vkCreateSemaphore(_vk->GetDevice(), &semaphoreCreateInfo, nullptr, &_semaphore);
	}

	~RenderPass()
	{

	}

	Texture& AddTextureInput(std::string name);
	Texture& AddTextureOutput(const std::string& name, const VkFormat format, const std::string& input = "");
	Texture& AddDepthOutput(const std::string& name);
	Buffer& AddBufferInput(std::string name);
	Buffer& AddBufferOutput(const std::string& name, unsigned size, void* data, const VkBufferUsageFlags usageFlags, const std::string& input = "");
	void AddDescriptorPoolSize(VkDescriptorPoolSize descriptorPoolSize) { _descriptorPoolSizes.push_back(std::move(descriptorPoolSize)); }
	void AddDescriptorSetLayoutBinding(VkDescriptorSetLayoutBinding descriptorSetLayoutBinding) { _descriptorSetLayoutBindings.push_back(std::move(descriptorSetLayoutBinding)); }



	Buffer& GetBuffer(const std::string& name) { return _graph.GetBufferResource(name); }

	// should have color attachments
	void Setup();
	void Execute();

	VkCommandBuffer& GetCommandBuffer() { return _commandBuffer; }
	VkPipeline& GetPipeline() { return _pipeline; }
	VkPipelineLayout& GetPipelineLayout() { return _pipelineLayout; }
	VkDescriptorSet& GetDescriptorSet() { return _descriptorSet; }
	VkSemaphore& GetSemaphore() { return _semaphore; }

	std::vector<std::shared_ptr<Buffer>>& GetBufferOutputs() { return _bufferOutputs; }

	std::vector<Renderable>& GetRenderables() { return _renderables; }

	void SetPipelineInfo(PipelineDescription pipelineDescription) { _pipelineDescription = std::move(pipelineDescription); }
	void SetDrawCalls(std::function<void(VkCommandBuffer&)> drawCalls) { _drawCalls = std::move(drawCalls); }
	void SetRenderables(std::vector<Renderable>& renderables) { _renderables = std::move(renderables); }
	void SetPushConstantRange(VkPushConstantRange pushConstantRange);
};

