#include "pch.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Window.h"
#include "GLTFLoader.h"
#include "VkBackend.h"
#include "RenderPass.h"

int main()
{
	//std::cout << std::filesystem::current_path() << std::endl;

	Window& win = Window::GetInstance();
	win.Init(800, 600, "Render Playground");

	VkBackend vk;
	vk.Init();
	GLTFLoader glLoader;

	GBufferPass gBufferPass;
	gBufferPass.model = glLoader.LoadModel("Models/Sponza/glTF/Sponza.gltf");
	gBufferPass.InitPass();


	//glMesh sponza = glLoader.LoadModel("Models/Sponza/glTF/Sponza.gltf");

	while (win.ProcessEvents())
	{
		//float time = GetTimeSeconds();

		VkCommandBuffer commandBuffer = vk.StartFrame();

	/*	color.texture = &swapchain[i];

		trianglePass.GetColorAttachment(0).texture = &swapchain[vk._vk.currentFrame];
		i++;
		if (i == 2)  i = 0;*/

		gBufferPass.Execute(commandBuffer);
		vk.EndFrame(commandBuffer);
	}

	return 0;
}