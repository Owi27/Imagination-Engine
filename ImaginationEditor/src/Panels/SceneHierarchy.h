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

		void SetSceneContext(const shared<Scene>& pScene) { _scene = pScene; }

		void DrawComponents(Entity* pEntity);

		void OnImGuiRender();
	};
}