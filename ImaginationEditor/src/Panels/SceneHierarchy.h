#pragma once
#include <Imgn/ImgnScene.h>

namespace Imgn
{
	class SceneHierarchy
	{
		shared<Scene> _scene;
		Entity* _selectedEntity = nullptr;

	public:
		SceneHierarchy() = default;
		SceneHierarchy(const shared<Scene>& pScene)
		{
			SetSceneContext(pScene);
		}

		void SetSceneContext(const shared<Scene>& pScene);

		void DrawComponents(Entity* pEntity);

		template<typename T>
		void CreateComponentSettings(Entity* pEntity, std::string_view pComponentTitle, std::function<void(bool)> pComponentSettings);

		void OnImGuiRender();
	};
}