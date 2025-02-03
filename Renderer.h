#pragma once
using namespace Microsoft::WRL;

class Renderer
{
protected:
	GWindow _win;
	unsigned int _width, _height, _swapchainImageCount;
	float _aspect;
	bool _isFocused = true;

	//time
	std::chrono::duration<float> _deltaTime;
	std::chrono::steady_clock::time_point _lastUpdate;

	//proxies
	GInput _gInput;
	GController _gController;


public:
	Renderer() = default;
	Renderer(GWindow win);
	~Renderer();

	virtual void Render() = 0;
	virtual void UpdateCamera() = 0;
};

class VulkanRenderer : public Renderer
{
	//gateware
	GVulkanSurface _vlk;
	GEventReceiver _shutdown;

	FrameGraph* _frameGraph = FrameGraph::GetInstance();
	GeometryData _geometryData;

	//structs
	//FrameBuffer _offscreenFrameBuffer;
	//UniformBufferOffscreen _offscreenData;
	//UniformBufferFinal _finalData;

	VkSemaphore _offscreenSe, _renderComplete;
	VkQueue _present;

	//vulkan
	VkDevice _device;
	VkPhysicalDevice _physicalDevice;
	VkRenderPass _renderPass;
	VkSampler _colorSampler;
//	Buffer _finalUniformBuffer, _geometryBuffer, _indexBuffer;
	VkDescriptorSet _offscreenDescriptorSet, _descriptorSet, _finalDescriptorSet;
	VkDescriptorPool _descriptorPool;
	VkDescriptorSetLayout _descriptorSetLayout;
	VkShaderModule _vertexShaderModule, _fragmentShaderModule, _offscreenVertexShaderModule, _offscreenFragmentShaderModule;
	//VkPipeline _offscreenPipeline, _graphicsPipeline;
	//VkPipelineLayout _pipelineLayout;
	std::vector<VkCommandBuffer> _drawCommandBuffers;
	VkCommandBuffer _offscreenCommandBuffer;
	VkCommandPool _commandPool;
	VkSemaphore _offscreenSemaphore, _compositionSemaphore, _postProcessSemaphore, _presentCompleteSemaphore;// , _renderCompleteSemaphore;
	VkSubmitInfo _submitInfo;
	VkQueue _queue;
	VkSwapchainKHR _swapchain;
	VkPipelineStageFlags _submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//VkPipelineCache _pipelineCache;

	std::vector<VkFence> _fences;
	const int MAX_FRAMES = 3;

	std::vector<DrawInfo> _drawInfo;
	Dimensions _dimensions;

	unsigned int _currentFrame = 0;

	std::vector<VkCommandBuffer> _commandBuffers[3];
	//dxc
	ComPtr<IDxcCompiler3> _compiler;
	ComPtr<IDxcUtils> _utils;
	ComPtr<IDxcIncludeHandler> _includeHandler;

	//tinygltf
	tinygltf::Model _model;

	//mat4 matrices[3];

	void CompileShaders();
	void LoadModel(std::string filename);
	void CreateGeometryData();
	//void CreateOffscreenFrameBuffer();
	//void CreateUniformBuffers();
	//void CreateDescriptorSets();
	//void WriteDescriptorSets();
	//void CreateGraphicsPipelines();
	//void CreateCommandBuffers();
	//void CreateDeferredCommandBuffers();
	void CreateFrameGraphNodes();
	//void UpdateLights();
	void CleanUp();
	void Prepare(FrameGraphNode node);
	template <typename T>
	bool DoesVectorContain(std::vector<T> v, T value) { return (std::find(v.begin(), v.end(), value) != v.end()); }
	VkWriteDescriptorSet MakeWrite(VkDescriptorSet descriptorSet, unsigned int binding, unsigned int descriptorCount, VkDescriptorType type, const VkDescriptorImageInfo* pImageInfo = nullptr, const VkDescriptorBufferInfo* pBufferInfo = nullptr);
	mat4 GetLocalMatrix(const tinygltf::Node& node);
	std::string ShaderAsString(const char* shaderFilePath);

public:
	VulkanRenderer(GWindow win);
	~VulkanRenderer();

	void Render() override;
	void UpdateCamera() override;
};

class DX12Renderer : public Renderer
{
	GDirectX12Surface _dxs;

public:
	DX12Renderer(GWindow win);
	~DX12Renderer();

	void Render() override;
	void UpdateCamera() override;
};