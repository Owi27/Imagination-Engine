#include "pch.h"
#include <glfw/include/GLFW/glfw3.h>
#include <iostream>
#include "ImgnWindow.h"
#include "ImgnVulkan.h"

int main()
{
	GWindow win;
	if (-win.Create(0, 0, 1280, 70, GWindowStyle::FULLSCREENBORDERED)) std::cout << "__FILE__ __LINE__: Window failed to be created\n";
	win.SetWindowName("Imagination Engine");

	ImgnVulkan imgnVulkan;
	imgnVulkan.InitVulkan(&win);

	while (+win.ProcessWindowEvents())
	{
		imgnVulkan.UpdateCamera();
		imgnVulkan.DrawFrame();
	}
	
	//std::unique_ptr<ImgnWindow> window = std::make_unique<ImgnWindow>(250, 250, 800, 600, L"Imagination Engine");
	//imgnVulkan.InitVulkan(window.get());

	//vec3<float> x = { 1, 0, 1 };
	//vec3<float> y = { 2, 0, 0 };

	//float z = Angle(x, y);
	//std::cout << z;

	/*while (window->ProcessMessages())
	{
		imgnVulkan.DrawFrame();
	}

	imgnVulkan.DeviceWaitIdle();*/

	return 0;
}