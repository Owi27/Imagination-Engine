#include "ImaginaryMath.hpp"
#include "RenderContext.h"
#include <memory>
#include <IWindow.h>
#include <VulkanBackend.h>
using namespace std;

int main()
{
	ImgnWindow& win = ImgnWindow::GetInstance();
	win.Init(800, 600, "Demo");

	VulkanBackend vk;
	vk.Init();

	while (win.ProcessEvents())
	{
		VkCommandBuffer commandBuffer = Attempt(vk.StartFrame());

		Attempt(vk.EndFrame(commandBuffer));
	}


	return 0;
}

// vin, 