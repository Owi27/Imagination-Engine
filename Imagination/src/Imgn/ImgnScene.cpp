#include "pch.hpp"
#include "ImgnScene.h"
#include "Components/ImgnComponents.h"

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

	Entity* Scene::CreateEntity(const std::string& pName)
	{
		_entities.emplace_back(Unique<Entity>(pName));
		_entities.back()->AddComponent<TransformComponent>();

		return &*_entities.back();
	}

	void Scene::DestroyEntity(unique<Entity>& pEntity)
	{
		std::erase(_entities, pEntity);
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