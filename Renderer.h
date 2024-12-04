#pragma once
using namespace Microsoft::WRL;

class Renderer
{
protected:
	GWindow _win;
	unsigned int _width, _height, _swapchainImageCount;
	float _aspect;

public:
	Renderer() = default;
	Renderer(GWindow win);
	~Renderer();

	virtual void Render() = 0;
};

class VulkanRenderer : public Renderer
{
	//gateware
	GVulkanSurface _vlk;

	//structs
	FrameBuffer _offscreenFrameBuffer;
	UniformBufferOffscreen _offscreenData;

	//vulkan
	VkDevice _device;
	VkPhysicalDevice _physicalDevice;
	VkRenderPass _renderPass;
	VkSampler _colorSampler;
	Buffer _uniformBuffer, _geometryBuffer;
	VkDescriptorSet _offscreenDescriptorSet, _descriptorSet;
	VkDescriptorPool _descriptorPool;
	VkDescriptorSetLayout _descriptorSetLayout;
	VkShaderModule _vertexShaderModule, _fragmentShaderModule, _offscreenVertexShaderModule, _offscreenFragmentShaderModule;
	VkPipeline _offscreenPipeline, _graphicsPipeline;
	VkPipelineLayout _pipelineLayout;
	std::vector<VkCommandBuffer> _drawCommandBuffers;
	VkCommandBuffer _offscreenCommandBuffer;
	VkCommandPool _commandPool;
	VkSemaphore _offscreenSemaphore;

	//dxc
	ComPtr<IDxcCompiler3> _compiler;
	ComPtr<IDxcUtils> _utils;
	ComPtr<IDxcIncludeHandler> _includeHandler;

	//tinygltf
	tinygltf::Model _model;

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
	std::string ShaderAsString(const char* shaderFilePath);

public:
	VulkanRenderer(GWindow win);
	~VulkanRenderer();

	void Render() override;
};

class DX12Renderer : public Renderer
{
	GDirectX12Surface _dxs;

public:
	DX12Renderer(GWindow win);
	~DX12Renderer();

	void Render() override;
};