#pragma once
#include "RenderPass.h"
class GeometryPass : public RenderPass
{
	RenderTarget* gBuffer;

public:
	GeometryPass(const std::string& pName) /*Constructor*/ : RenderPass(pName)
	{
		gBuffer = new RenderTarget(1920, 1080);

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

