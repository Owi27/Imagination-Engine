#include "pch.h"
#include "VulkanContext.h"

VulkanContext::VulkanContext(GWindow win)
{
#ifndef NDEBUG
	const char* debugLayers[] =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	if (+_vulkanSurface.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT | GW::GRAPHICS::TRIPLE_BUFFER, sizeof(debugLayers) / sizeof(debugLayers[0]), debugLayers, 0, nullptr, 0, nullptr, false))
#else
	if (+_vulkanSurface.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT | GW::GRAPHICS::TRIPLE_BUFFER))
#endif
	{
		_vulkanSurface.GetDevice((void**)&_device);
		_vulkanSurface.GetPhysicalDevice((void**)&_physicalDevice);
		_vulkanSurface.GetCommandPool((void**)&_commandPool);
		_vulkanSurface.GetGraphicsQueue((void**)&_graphicsQueue);
		_vulkanSurface.GetSwapchain((void**)&_swapchain);

		//dxc
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
		_utils->CreateDefaultIncludeHandler(&_includeHandler);
		std::filesystem::create_directories("Shaders/SPV");



	}
}

VkFramebuffer VulkanContext::GetFrameBuffer(int idx)
{
	_vulkanSurface.GetSwapchainFramebuffer(idx, (void**)&_frameBuffer);
	return _frameBuffer;
}
