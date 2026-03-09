#include "pch.h"
#include <glfw/include/GLFW/glfw3.h>
#include <iostream>
#include "ImgnWindow.h"
#include "ImgnVulkan.h"

int main()
{
	std::cout << "Creating Window\n";

	std::unique_ptr<ImgnWindow> window = std::make_unique<ImgnWindow>(250, 250, 800, 600, L"Imagination Engine");
	ImgnVulkan imgnVulkan;
	imgnVulkan.InitVulkan(window.get());

	while (window->ProcessMessages())
	{
		imgnVulkan.DrawFrame();
	}

	imgnVulkan.DeviceWaitIdle();

	return 0;
}