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

	//structs
	FrameBuffer _offscreenFrameBuffer;
	UniformBufferOffscreen _offscreenData;
	UniformBufferFinal _finalData;

	//vulkan
	VkDevice _device;
	VkPhysicalDevice _physicalDevice;
	VkRenderPass _renderPass;
	VkSampler _colorSampler;
	Buffer _offscreenUniformBuffer, _finalUniformBuffer, _geometryBuffer;
	VkDescriptorSet _offscreenDescriptorSet, _descriptorSet, _finalDescriptorSet;
	VkDescriptorPool _descriptorPool;
	VkDescriptorSetLayout _descriptorSetLayout;
	VkShaderModule _vertexShaderModule, _fragmentShaderModule, _offscreenVertexShaderModule, _offscreenFragmentShaderModule;
	VkPipeline _offscreenPipeline, _graphicsPipeline;
	VkPipelineLayout _pipelineLayout;
	std::vector<VkCommandBuffer> _drawCommandBuffers;
	VkCommandBuffer _offscreenCommandBuffer;
	VkCommandPool _commandPool;
	VkSemaphore _offscreenSemaphore, _presentCompleteSemaphore, _renderCompleteSemaphore;
	VkSubmitInfo _submitInfo;
	VkQueue _queue;
	VkSwapchainKHR _swapchain;
	VkPipelineStageFlags _submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	//dxc
	ComPtr<IDxcCompiler3> _compiler;
	ComPtr<IDxcUtils> _utils;
	ComPtr<IDxcIncludeHandler> _includeHandler;

	//tinygltf
	tinygltf::Model _model;

	//mat4 matrices[3];

	void CompileShaders();
	void LoadModel(std::string filename);
	void CreateGeometryBuffer();
	void CreateOffscreenFrameBuffer();
	void CreateUniformBuffers();
	void CreateDescriptorSets();
	void WriteDescriptorSets();
	void CreateGraphicsPipelines();
	void CreateCommandBuffers();
	void CreateDeferredCommandBuffers();
	void UpdateLights();
	void CleanUp();
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