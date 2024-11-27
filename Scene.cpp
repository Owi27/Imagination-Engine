#include "pch.h"
#include "Scene.h"

namespace ECS
{
	Scene::Scene()
	{
		entt::entity entity = _registry.create();
	}

	Scene::~Scene()
	{
	}
}