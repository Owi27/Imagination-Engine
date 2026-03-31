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

	/*Copy Constructor*/
	GeometryPass(const GeometryPass& pOther) = default;

	/*Copy Assignment Operator*/
	GeometryPass& operator=(const GeometryPass& pOther) = default;

	/*Move Constructor*/
	GeometryPass(GeometryPass&& pOther) noexcept = default;

	/*Move Assignment Operator*/
	GeometryPass& operator=(GeometryPass&& pOther) noexcept = default;
};