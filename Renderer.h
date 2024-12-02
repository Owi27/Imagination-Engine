#pragma once
class Renderer
{
protected:
	GWindow _win;
	unsigned int _width, _height;
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
	VkSampler _colorSampler;
	Buffer _uniformBuffer;


	void CreateOffscreenFrameBuffer();
	void CreateUniformBuffers();

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