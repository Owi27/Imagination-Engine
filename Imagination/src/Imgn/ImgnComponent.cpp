#include "pch.hpp"
#include "ImgnComponent.h"

namespace Imgn
{
	ID GenerateEntityID(std::string_view pName)
	{
		static uint64_t counter = 0;

		std::string id = std::string(pName) + std::to_string(counter++);

		return HashID(id);
	}

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