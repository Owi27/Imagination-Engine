#pragma once
enum class RendererBackend
{
	Vulkan, D3D12, Metal
};

struct RendererCreateInfo
{
	RendererBackend backend = RendererBackend::Vulkan;

	void* windowHandle = nullptr, * displayHandle = nullptr;

	uint32_t width = 1280, height = 720;

	bool enableValidation = true;
};

class IRenderBackend
{


public:
	/* Class Defaults */
	IRenderBackend()
	{

	}

	~IRenderBackend()
	{

	}

	/* Class Functions */
	virtual void Init(RendererCreateInfo pCreateInfo) = 0;
};