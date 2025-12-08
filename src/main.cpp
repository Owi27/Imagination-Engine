#include "ImaginaryMath.hpp"
#include "RenderContext.h"
#include <memory>
#include <IWindow.h>
#include <VulkanBackend.h>
#include "Passes.h"
using namespace std;

struct Vertex
{
	float x, y;
	std::array<unsigned char, 4> col;

	void AddToColorBuffer(std::vector<unsigned char>& pColorBuffer)
	{
		pColorBuffer.insert(pColorBuffer.end(), col.begin(), col.end());
	}
};

//float GetTimeSeconds()
//{
//	using clock = std::chrono::steady_clock;
//	static auto start = clock::now();                // record start once
//	auto now = clock::now();
//	std::chrono::duration<float> elapsed = now - start;
//	return elapsed.count();                          // seconds since start
//}

int main()
{
	ImgnWindow& win = ImgnWindow::GetInstance();
	win.Init(800, 600, "Demo");

	VulkanBackend vk;
	vk.Init();

	Vertex v1 = { 0.f, -.5f, {255, 183, 197, 255} }, v2 = { .5f, .5f, {255, 183, 255, 255} }, v3 = { -.5f, .5f, {255, 255, 197, 255} };
	//Vertex v1 = { 0.f, -5.f, 0xFF, 0x00, 0x00, 0xFF }, v2 = { 5.f, 5.f, 0x00, 0xFF, 0x00, 0xFF }, v3 = { -5.f, 5.f, 0x00, 0x00, 0xFF, 0xFF };

	Vertex vsp[] = { v1, v2, v3 };

	float pos[] = { v1.x, v1.y, v2.x, v2.y, v3.x, v3.y };
	std::vector<unsigned char> colorBytes;
	//v1.AddToColorBuffer(colorBytes);
	//v2.AddToColorBuffer(colorBytes);
	//v3.AddToColorBuffer(colorBytes);

	for (auto& v : vsp)
	{
		v.AddToColorBuffer(colorBytes);
	}

	Buffer posB(vk._vk), colB(vk._vk);


	VkBuffer bs[] = { posB.buffer, colB.buffer };
	VkDeviceSize o[] = { 0 };

	Buffer vertex(vk._vk);
	vertex.CreateBuffer(sizeof(Vertex) * 3, BufferUsage::VERTEX, MemoryFlags::CPU | MemoryFlags::CPU2GPU).WriteToBuffer(vsp);

	Texture tex(vk._vk);
	tex.LoadImage("../Tex/red.png");

	TrianglePass trianglePass(vk._vk);
	GBufferPass gBufferPass(vk._vk);

	posB.CreateBuffer(sizeof(float) * 2 * 3, BufferUsage::VERTEX, MemoryFlags::CPU | MemoryFlags::CPU2GPU).WriteToBuffer(pos);
	colB.CreateBuffer(sizeof(unsigned char) * 4 * 3, BufferUsage::VERTEX, MemoryFlags::CPU | MemoryFlags::CPU2GPU).WriteToBuffer(colorBytes.data());

	trianglePass.pos = std::move(posB);
	trianglePass.col = std::move(colB);

	std::array<Texture, 3> swapchain = { Texture(vk._vk), Texture(vk._vk), Texture(vk._vk) };
	swapchain[0].Swapchain(vk._vk.swapchainImages[0], vk._vk.swapchainImageViews[0], PipelineFormat::SWAPCHAIN, { vk._vk.win.GetWidth(), vk._vk.win.GetHeight(), 1 }, ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
	swapchain[1].Swapchain(vk._vk.swapchainImages[1], vk._vk.swapchainImageViews[1], PipelineFormat::SWAPCHAIN, { vk._vk.win.GetWidth(), vk._vk.win.GetHeight(), 1 }, ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
	swapchain[2].Swapchain(vk._vk.swapchainImages[2], vk._vk.swapchainImageViews[2], PipelineFormat::SWAPCHAIN, { vk._vk.win.GetWidth(), vk._vk.win.GetHeight(), 1 }, ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
	
	int i = 0;
	//vk.GetCurrentFrame(swap);
	AttachmentDesc color
	{
		.texture = &swapchain[i],
		.loadOp = LoadOp::CLEAR,
		.storeOp = StoreOp::STORE,
		.clearValue
		{
			.color = {{ 0.f, 0.f, 0.f, 1.f }},
			//.depthStencil = ,
		}
	};

	Texture dep(vk._vk);
	dep.Swapchain(vk._vk.depthImage, vk._vk.depthImageView, vk._vk.depthFormat, { vk._vk.swapchainExtent.width, vk._vk.swapchainExtent.height, 1 }, ImageAspect::DEPTH);
	
	AttachmentDesc depth
	{
		.texture = &dep,
		.loadOp = LoadOp::CLEAR,
		.storeOp = StoreOp::TRASH,
		.clearValue
		{
			//.color = ,
			.depthStencil = { 1.f, 0 },
		}
	};

	trianglePass.AddColorAttachment(color);
	//trianglePass.AddColorAttachment(s2);
	//trianglePass.AddColorAttachment(s3);
	trianglePass.SetDepthAttachment(depth);

	while (win.ProcessEvents())
	{
		//float time = GetTimeSeconds();

		VkCommandBuffer commandBuffer = Attempt(vk.StartFrame());

		color.texture = &swapchain[i];

		trianglePass.GetColorAttachment(0).texture = &swapchain[vk._vk.currentFrame];
		i++;
		if (i == 2)  i = 0;

		trianglePass.Execute(commandBuffer);

		Attempt(vk.EndFrame(commandBuffer));
	}

	return 0;
}