#include "D:/GitHub/Imagination-Engine/Refactor/Imagination - Current/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
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
