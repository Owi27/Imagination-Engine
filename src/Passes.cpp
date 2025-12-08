#include "D:/GitHub/Imagination-Engine/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
#include "Passes.h"

float GetTimeSeconds()
{
	using clock = std::chrono::steady_clock;
	static auto start = clock::now();                // record start once
	auto now = clock::now();
	std::chrono::duration<float> elapsed = now - start;
	return elapsed.count();                          // seconds since start
}

void TrianglePass::Record(VkCommandBuffer pCommandBuffer)
{
	float time = GetTimeSeconds();
	VkDeviceSize offset = 0;

	vkCmdBindPipeline(pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
	vkCmdPushConstants(pCommandBuffer, _pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &time);
	vkCmdBindVertexBuffers(pCommandBuffer, 0, 1, &pos.buffer, &offset);
	vkCmdBindVertexBuffers(pCommandBuffer, 1, 1, &col.buffer, &offset);

	// if you still want push constants for time or whatever:

	vkCmdDraw(pCommandBuffer, 3, 1, 0, 0);
}

void GBufferPass::Record(VkCommandBuffer pCommandBuffer)
{
}