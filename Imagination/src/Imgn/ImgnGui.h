#pragma once
#include "ImgnComponent.h"
#include "ImgnApp.hpp"

class ImGuiComponent : public ImgnComponent
{
public:
	ImGuiComponent() : ImgnComponent("ImGuiComponent")
	{

	}

	~ImGuiComponent()
	{

	}

	virtual void OnInit();
	virtual void Dream(float pDeltaTime);
	virtual void OnEvent(Event& pEvent);
	virtual void OnDestroy();

};

