#include "pch.hpp"
#include "ImgnComponent.h"

void ImgnEntity::Init()
{
	for (auto& component : _components)
	{
		component->Init();
	}
}

void ImgnEntity::Dream(float pDeltaTime)
{
	if (!_active) return;

	for (auto& component : _components)
	{
		component->Dream(pDeltaTime);
	}
}

void ImgnEntity::OnEvent(Event& pEvent)
{
	if (!_active) return;

	for (auto& component : _components)
	{
		component->OnEvent(pEvent);

		if (pEvent.Handled()) break;
	}
}

void ImgnEntity::Render()
{
	if (!_active) return;

	for (auto& component : _components)
	{
		component->Render();
	}
}
