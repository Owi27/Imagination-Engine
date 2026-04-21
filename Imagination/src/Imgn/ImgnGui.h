#pragma once
#include "ImgnComponent.h"

class ImGuiComponent : public ImgnComponent
{
public:
	ImGuiComponent() : ImgnComponent("ImGuiComponent")
	{

	}

	~ImGuiComponent()
	{

	}

	virtual void Dream(float pDeltaTime);
	virtual void OnEvent(Event& pEvent);
};

