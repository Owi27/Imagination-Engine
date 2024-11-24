#pragma once
using namespace Microsoft::WRL;

class Renderer
{
	//gateware
	GWindow _win;
	GVulkanSurface _vlk;
	GEventReceiver _shutdown;

	//vulkan
	VkDevice _device = nullptr;
	VkPhysicalDevice _physicalDevice = nullptr;
	VkQueue _graphicsQueue = nullptr;
	VkCommandPool _commandPool = nullptr;
	VkShaderModule _vertexShaderModule = nullptr;
	VkShaderModule _pixelShaderModule = nullptr;
	VkPipeline _graphicsPipeline = nullptr;
	VkPipelineLayout _graphicsPipelineLayout = nullptr;
	VkRenderPass _renderPass = nullptr;
	//shader modules
	VkShaderModule _vertexShader, _pixelShader;
	//Buffers
	Buffer _vertexBuffer, _normalBuffer, _texCoordBuffer, _idxBuffer, _materialBuffer, _primInfo, _sceneDesc;
	//Texture _textureBuffer;
	std::vector<Texture> _textures;
	Texture _depthTexture;
	//std::vector<glMaterial> _materials;
	//uniform buffer
	std::vector<Buffer> _uniformBuffers;
	std::vector<VkDescriptorSet> _descriptorSets;
	VkDescriptorSetLayout _descriptorSetLayout;
	VkDescriptorPool _descriptorPool;

	Matrices _matrices;
	mat4 world = GW::MATH::GIdentityMatrixF, view = GW::MATH::GIdentityMatrixF, proj = GW::MATH::GIdentityMatrixF;

	bool _isFocused = true;

	//time
	std::chrono::duration<float> _deltaTime;
	std::chrono::steady_clock::time_point _lastUpdate;

	//floats & ints
	unsigned int _width, _height, _frames, _mipLevels;
	float _aspect;

	//proxies
	GInput _gInput;
	GController _gController;

	//dxc
	ComPtr<IDxcCompiler3> _compiler;
	ComPtr<IDxcUtils> _utils;
	ComPtr<IDxcIncludeHandler> _includeHandler;

	//tiny
	tinygltf::Model _model;
	GLTFScene _gltfScene;

	/* FUNCTIONS */
	void InitDXC();
	void CompileShaders();
	void LoadModel(std::string filename, float scale = 1.f);
	void CreateTextureImages(VkCommandBuffer& commandBuffer);
	void CreateGeometryBuffer();
	void CreateUniformBuffers();
	void CreateDescriptorSets();
	//void CreateDepthTexure(unsigned int index);
	void CreateGraphicsPipeline();
	void WriteDescriptorSets();
	void UploadTextureToGPU(const char* filepath, Texture* texture);
	void UploadTextureToGPU(tinygltf::Image* img, Texture* texture);
	void CreateSampler(Texture* texture);
	void CleanUp();
	std::string ShaderAsString(const char* shaderFilePath);

public:
	Renderer(GWindow win, GVulkanSurface vlk);
	~Renderer();

	void Render();
	void UpdateCamera();
};

