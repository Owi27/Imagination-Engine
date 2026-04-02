#pragma once
#include "RenderPass.h"
class GeometryPass : public RenderPass
{
	RenderTarget* gBuffer;
	//std::array<RenderTarget*, 3> gBufferTargets;

public:
	GeometryPass(const std::string& pName) /*Constructor*/ : RenderPass(pName)
	{
		gBuffer = new RenderTarget(1920, 1080);

		//for (auto& target : gBufferTargets)
		//{
		//	target = new RenderTarget(1920, 1080);
		//}

		SetRenderTarget(gBuffer);
	}

	~GeometryPass() override/*Destructor*/
	{
		delete gBuffer;
	}

	// Inherited via RenderPass
	void BeginPass(vk::raii::CommandBuffer& pCommandBuffer) override;
	
	void Render(vk::raii::CommandBuffer& pCommandBuffer) override;

	void EndPass(vk::raii::CommandBuffer& pCommandBuffer) override;
};

