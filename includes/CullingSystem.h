#pragma once
#include "Camera.h"

class CullingSystem
{
	Camera* _camera;
	std::vector<Entity*> _visibleEntities;

public:
	explicit CullingSystem(Camera* pCamera)
	{
		_camera = pCamera;
	}

	void SetCamera(Camera* pCamera)
	{
		_camera = pCamera;
	}

	void CullScene(const std::vector<Entity*>& pAllEntities);
};

