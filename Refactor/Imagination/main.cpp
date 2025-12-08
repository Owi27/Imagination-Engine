#include "pch.h"
#include "Window.h"
#include "VkBackend.h"
#include "RenderPass.h"

int main()
{
	Window& win = Window::GetInstance();
	win.Init(800, 600, "Render Playground");

	VkBackend vk;
	vk.Init();

	TrianglePass trianglePass;

	while (win.ProcessEvents())
	{
		//float time = GetTimeSeconds();

		VkCommandBuffer commandBuffer = vk.StartFrame();

	/*	color.texture = &swapchain[i];

		trianglePass.GetColorAttachment(0).texture = &swapchain[vk._vk.currentFrame];
		i++;
		if (i == 2)  i = 0;*/

		trianglePass.Execute(commandBuffer);
		vk.EndFrame(commandBuffer);
	}

	return 0;
}