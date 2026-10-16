#include "pch.hpp"
#include "ImgnScene.h"
#include "Components/AllComponents.hpp"

namespace Imgn
{
	void Scene::Dream(Time pTime)
	{
		//update scripts
		{
			for (unique<Entity>& entity : _entities)
			{
				if (ScriptComponent* scriptComponent = entity->GetComponent<ScriptComponent>())
				{
					if (!scriptComponent->instance)
					{
						scriptComponent->Create();
						scriptComponent->instance->_entity = entity.get();
						scriptComponent->Sleep();
					}

					scriptComponent->Dream(pTime);
				}
			}
		}
	}

	Entity* Scene::CreateEntity(const std::string& pName, ID pID)
	{
		if (pID != 0 && GetEntity(pID))
		{
			IMGN_CORE_ERROR("Cannot create entity '{}' with duplicate ID {}.", pName, pID);
			return nullptr;
		}

		_entities.emplace_back(Unique<Entity>(pName, pID));

		Entity* entity = _entities.back().get();
		entity->AddComponent<TransformComponent>();

		return entity;
	}

	void Scene::DestroyEntity(Entity* pEntity)
	{
		if (!pEntity) return;

		pEntity->RemoveFromParent();

		std::vector<Entity*> children = pEntity->GetChildren();

		for (Entity* child : children) pEntity->RemoveChild(child);

		std::erase_if(_entities, [pEntity](const unique<Entity>& pEntityEntry) { return pEntityEntry.get() == pEntity; });
	}

	void Scene::DestroyEntity(unique<Entity>& pEntity)
	{
		std::erase(_entities, pEntity);
	}

	Entity* Scene::GetEntity(ID pID)
	{
		if (pID == 0) return nullptr;

		for (const unique<Entity>& entity : _entities)
		{
			if (entity->GetID() == pID) return entity.get();
		}

		return nullptr;
	}

	Entity* Scene::GetPrimaryCameraEntity()
	{
		for (auto& entity : _entities)
		{
			if (CameraComponent* cameraComponent = entity->GetComponent<CameraComponent>())
			{
				if (cameraComponent->mainCamera)
				{
					return entity.get();
				}
			}
		}

		return nullptr;
	}

	void Scene::OnViewportResize(uint32_t pWidth, uint32_t pHeight)
	{
		_viewportWidth = pWidth;
		_viewportHeight = pHeight;

		for (auto& entity : _entities)
			if (CameraComponent* cameraComp = entity->GetComponent<CameraComponent>())
				if (!cameraComp->fixedAspect) cameraComp->camera.SetViewportSize(pWidth, pHeight);
	}

}