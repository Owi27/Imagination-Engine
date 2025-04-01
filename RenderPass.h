#pragma once
class Buffer;
class Texture;
class FrameGraph;
enum FrameGraphQueueBit;

class RenderPass
{
	VulkanContext& _vk;
	FrameGraph& _graph;
	std::string _name;
	FrameGraphQueueBit _queue;
	std::vector<Texture*> _colorInputs;
	std::vector<Texture*> _colorOutputs;
	std::vector<Buffer*> _bufferInputs;
	std::vector<Buffer*> _bufferOutputs;
	Texture* _depthStencilInput = nullptr;
	Texture* _depthStencilOutput = nullptr;

	Buffer* _uniformBufferOutput;

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

	std::vector<std::shared_ptr<Shader>> _shaders;

	std::string _ubDataName;

	bool _renderToSwapchain = false;

	bool _firstRun = true;

public:
	RenderPass(FrameGraph& graph, FrameGraphQueueBit queue) : _vk(*VulkanContext::GetInst()), _graph(graph), _queue(queue)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		vkCreateSemaphore(_vk.GetDevice(), &semaphoreCreateInfo, nullptr, &_semaphore);

		VkCommandBufferAllocateInfo commandBufferAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = _vk.GetCommandPool(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};

		vkAllocateCommandBuffers(_vk.GetDevice(), &commandBufferAllocateInfo, &_commandBuffer);
	}

	~RenderPass()
	{

	}

	Texture& AddTextureInput(std::string name);
	void AddTOutput(const std::string& name, VkFormat format = VK_FORMAT_UNDEFINED);
	void AddDOutput(const std::string& name);
	void AddTInput(const std::string& name);

	//template<typename T>
	void AddUB(const std::string& name, void* data, unsigned size);
	//template<typename T>
	void AddVBOutput(const std::string& name, void* data, unsigned size);
	//template<typename T>
	void AddIBOutput(const std::string& name, void* data, unsigned size);
	//template<typename T>
	void UpdateUB(const std::string& name);

	Texture& AddTextureOutput(const std::string& name, const VkFormat format, const std::string& input = "");
	Texture& AddDepthOutput(const std::string& name);
	Buffer& AddBufferInput(std::string name);
	Buffer& AddBufferOutput(const std::string& name, unsigned size, void* data, const VkBufferUsageFlags usageFlags, const std::string& input = "");
	Buffer& AddUniformBufferOutput(const std::string& name, unsigned size, void* data, const VkBufferUsageFlags usageFlags, const std::string& input = "");
	void AddDescriptorPoolSize(VkDescriptorPoolSize descriptorPoolSize) { _descriptorPoolSizes.push_back(std::move(descriptorPoolSize)); }
	void AddDescriptorSetLayoutBinding(VkDescriptorSetLayoutBinding descriptorSetLayoutBinding) { _descriptorSetLayoutBindings.push_back(std::move(descriptorSetLayoutBinding)); }

	Buffer& GetBuffer(const std::string& name);

	// should have color attachments
	void Setup();
	void Execute();

	VkCommandBuffer& GetCommandBuffer() { return _commandBuffer; }
	VkPipeline& GetPipeline() { return _pipeline; }
	VkPipelineLayout& GetPipelineLayout() { return _pipelineLayout; }
	VkDescriptorSet& GetDescriptorSet() { return _descriptorSet; }
	VkSemaphore& GetSemaphore() { return _semaphore; }

	std::vector<Buffer*>& GetBufferOutputs() { return _bufferOutputs; }

	std::vector<Renderable>& GetRenderables() { return _renderables; }

	void SetName(const std::string& name) { _name = name; }
	//void SetPipelineInfo(PipelineDescription pipelineDescription) { _pipelineDescription = std::move(pipelineDescription); }
	void SetDrawCalls(std::function<void(VkCommandBuffer&)> drawCalls) { _drawCalls = std::move(drawCalls); }
	void SetRenderables(std::vector<Renderable>& renderables) { _renderables = std::move(renderables); }
	void SetPushConstantRange(VkPushConstantRange pushConstantRange);
	void SetShaders(const std::string& shaderName = "");
	void SetComputeShader(const std::string& shaderName);
	void SetVertexInput(unsigned vertexInputs) { _pipelineDescription.vertexInput = vertexInputs; }
	void SetCullMode(VkCullModeFlags cullMode) { _pipelineDescription.cullMode = cullMode; }
};