#include "ImaginaryMath.hpp"
#include "RenderContext.h"
#include <memory>
#include <IWindow.h>
#include <VulkanBackend.h>
using namespace std;

struct Vertex
{
	float x, y;
	unsigned char r, g, b, a;
};

float GetTimeSeconds()
{
	using clock = std::chrono::steady_clock;
	static auto start = clock::now();                // record start once
	auto now = clock::now();
	std::chrono::duration<float> elapsed = now - start;
	return elapsed.count();                          // seconds since start
}

int main()
{
	ImgnWindow& win = ImgnWindow::GetInstance();
	win.Init(800, 600, "Demo");

	VulkanBackend vk;
	vk.Init();

	//Vertex v1 = { 0.f, -.5f, 0xFF, 0x00, 0x00, 0xFF }, v2 = { .5f, .5f, 0x00, 0xFF, 0x00, 0xFF }, v3 = { -.5f, .5f, 0x00, 0x00, 0xFF, 0xFF };
	Vertex v1 = { 0.f, -5.f, 0xFF, 0x00, 0x00, 0xFF }, v2 = { 5.f, 5.f, 0x00, 0xFF, 0x00, 0xFF }, v3 = { -5.f, 5.f, 0x00, 0x00, 0xFF, 0xFF };

	Vertex vsp[] = { v1, v2, v3 };

	float pos[] = { v1.x, v1.y, v2.x, v2.y, v3.x, v3.y };
	unsigned char col[] = { v1.r, v1.g, v1.b, v1.a, v2.r, v2.g, v2.b, v2.a , v3.r, v3.g, v3.b, v3.a };

	Buffer posB(vk._vk), colB(vk._vk);
	posB.CreateBuffer(sizeof(float) * 2 * 3, BufferUsage::VERTEX, MemoryFlags::CPU | MemoryFlags::BOTH).WriteToBuffer(pos);
	colB.CreateBuffer(sizeof(unsigned char) * 4 * 3, BufferUsage::VERTEX, MemoryFlags::CPU | MemoryFlags::BOTH).WriteToBuffer(col);

	VkBuffer bs[] = { posB.buffer, colB.buffer };
	VkDeviceSize o[] = { 0 };

	Buffer vertex(vk._vk);
	vertex.CreateBuffer(sizeof(Vertex) * 3, BufferUsage::VERTEX, MemoryFlags::CPU | MemoryFlags::BOTH).WriteToBuffer(vsp);

	while (win.ProcessEvents())
	{
		float time = GetTimeSeconds();

		VkCommandBuffer commandBuffer = Attempt(vk.StartFrame());

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk._vk.pipeline);
		vkCmdPushConstants(commandBuffer, vk._vk.pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &time);
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &posB.buffer, o);
		vkCmdBindVertexBuffers(commandBuffer, 1, 1, &colB.buffer, o);

		vkCmdDraw(commandBuffer, /*vertexCount*/ 3, /*instanceCount*/ 1, /*firstVertex*/ 0, /*firstInstance*/ 0);

		Attempt(vk.EndFrame(commandBuffer));
	}

	return 0;
}