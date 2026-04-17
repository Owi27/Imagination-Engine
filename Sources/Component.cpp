#include "pch.h"
#include "Component.h"

void Entity::Init()
{
	for (auto& component : _components)
	{
		component->Init();
	}
}

void Entity::Update(float pDeltaTime)
{
	if (!_active) return;

	for (auto& component : _components)
	{
		component->Update(pDeltaTime);
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
