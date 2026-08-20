#include "pch.hpp"
#include "ImgnComponent.h"

namespace Imgn
{
	void Entity::Init()
	{
		for (auto& component : _components)
		{
			component->Init();
		}
	}

	void Entity::Dream(float pDeltaTime)
	{
		if (!_active) return;

		for (auto& component : _components)
		{
			component->Dream(pDeltaTime);
		}
	}

	void Entity::OnEvent(Event& pEvent)
	{
		if (!_active) return;

		for (auto& component : _components)
		{
			component->OnEvent(pEvent);

			if (pEvent.Handled()) break;
		}
	}

	void Entity::Render()
	{
		if (!_active) return;

		for (auto& component : _components)
		{
			component->Render();
		}
	}
}