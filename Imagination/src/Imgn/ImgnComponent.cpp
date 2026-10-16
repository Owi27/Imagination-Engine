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

	Entity* Entity::AddChild(Entity* pChild)
	{
		if (!pChild || pChild == this) return nullptr;

		for (Entity* parent = this; parent; parent = parent->_parent) if (parent == pChild) return nullptr;

		if (pChild->_parent == this) return pChild;

		if (pChild->_parent) pChild->_parent->RemoveChild(pChild);

		pChild->_parent = this;
		_children.push_back(pChild);

		return pChild;
	}

	bool Entity::RemoveChild(Entity* pChild)
	{
		if (!pChild) return false;

		const size_t removed = std::erase(_children, pChild);
		if (removed == 0) return false;

		if (pChild->_parent == this) pChild->_parent = nullptr;

		return true;
	}

	void Entity::RemoveFromParent()
	{
		if (!_parent) return;

		_parent->RemoveChild(this);
	}
}