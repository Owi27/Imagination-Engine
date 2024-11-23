#include "pch.h"
#include "Renderer.h"

void main()
{
	GWindow win;
	GEventResponder msgs;
	GVulkanSurface vulkan;
	if (+win.Create(0, 0, 800, 600, GWindowStyle::WINDOWEDBORDERED))
	{
		VkClearValue clrAndDepth[2];
		clrAndDepth[0].color = { {0, 0, 0, 1} }; // start with a black screen
		clrAndDepth[1].depthStencil = { 1.0f, 0u };

		msgs.Create([&](const GW::GEvent& e)
			{
				GW::SYSTEM::GWindow::Events q;
				if (+e.Read(q) && q == GWindow::Events::RESIZE)
					clrAndDepth[0].color.float32[2] += 0.01f; // move towards a yellow as they resize
			});

		win.Register(msgs);

#ifndef NDEBUG
		const char* debugLayers[] =
		{
			"VK_LAYER_KHRONOS_validation"
		};

		std::vector<const char*> deviceExtensions =
		{
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
			VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME
		};

		if (+vulkan.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT, sizeof(debugLayers) / sizeof(debugLayers[0]), debugLayers, 0, nullptr, deviceExtensions.size(), deviceExtensions.data(), true))
#else
		if (+vulkan.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
#endif
		{
			Renderer renderer(win, vulkan);
			while (+win.ProcessWindowEvents())
			{
				if (+vulkan.StartFrame(2, clrAndDepth))
				{
					renderer.UpdateCamera();
					renderer.Render();
					vulkan.EndFrame(true);
				}
			}
		}
	}

}