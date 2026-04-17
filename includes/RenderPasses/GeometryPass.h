#pragma once
#include "RenderPass.h"

struct UBO
{
	mat4 model, view, proj;
};

class GeometryPass : public RenderPass
{
	RenderTarget* gBuffer;

	
	//std::array<RenderTarget*, 3> gBufferTargets;

public:
	GeometryPass(const std::string& pName) /*Constructor*/ : RenderPass(pName)
	{
		gBuffer = new RenderTarget(1920, 1080);
		SetRenderTarget(gBuffer);

		//descriptor set layout
		{

		}
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

