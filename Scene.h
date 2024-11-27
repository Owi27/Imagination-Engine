#pragma once
namespace ECS
{
	class Scene
	{
		entt::registry _registry;

	public:
		Scene();
		~Scene();
	};


}