#pragma once

enum class eRendererBackend
{
	Vulkan, D3D12, Metal
};

#if defined(IMGN_METAL_PLATFORM_MACOS)
inline constexpr eRendererBackend DefaultRendererBackend = eRendererBackend::Metal;
#else
inline constexpr eRendererBackend DefaultRendererBackend = eRendererBackend::Vulkan;
#endif

struct RendererCreateInfo
{
	eRendererBackend backend = DefaultRendererBackend;

	void* windowHandle = nullptr, * displayHandle = nullptr;

	uint32_t width = 1280, height = 720;

	bool enableValidation = true;
};

class IRenderBackend
{
public:
	virtual ~IRenderBackend() = default;

	/* Class Functions */
	virtual bool Sleep(RendererCreateInfo pCreateInfo) = 0;
	virtual void Dream() = 0;
	virtual void WakeUp() = 0;
};