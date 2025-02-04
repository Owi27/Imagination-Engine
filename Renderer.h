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

	VkQueue _present;

	//vulkan
	VkDevice _device;
	VkPhysicalDevice _physicalDevice;
	VkRenderPass _renderPass;
	VkSampler _colorSampler;
	VkDescriptorSet _offscreenDescriptorSet, _descriptorSet, _finalDescriptorSet;
	VkDescriptorPool _descriptorPool;
	VkDescriptorSetLayout _descriptorSetLayout;
	VkShaderModule _vertexShaderModule, _fragmentShaderModule, _offscreenVertexShaderModule, _offscreenFragmentShaderModule;
	std::vector<VkCommandBuffer> _drawCommandBuffers;
	VkCommandBuffer _offscreenCommandBuffer;
	VkCommandPool _commandPool;
	VkSemaphore _offscreenSemaphore, _compositionSemaphore, _postProcessSemaphore, _presentCompleteSemaphore[3];// , _renderCompleteSemaphore;
	VkSubmitInfo _submitInfo;
	VkQueue _queue;
	VkSwapchainKHR _swapchain;
	VkPipelineStageFlags _submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

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
	void CreateFrameGraphNodes();
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