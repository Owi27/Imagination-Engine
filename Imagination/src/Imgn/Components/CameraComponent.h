#pragma once
#include "Imgn/ImgnComponent.h"
#include "Imgn/ImgnCamera.h"

namespace Imgn
{
	class IMGN_API CameraComponent : public Component
	{
	public:
		IMGN_COMPONENT_ID("Imgn.CameraComponent");

		Camera camera;
		bool mainCamera = true;
		bool fixedAspect = false;
		float cameraSpeed = 250.f;

		CameraComponent(float pSizeOrFOV = Math::Radians(45.f), float pNearPlane = .1f, float pFarPlane = 5000.f, CameraType pType = CameraType::Perspective) : Component("Camera")
		{
			switch (pType)
			{
			case Imgn::CameraType::Perspective:
				camera.CreatePerspectiveCamera(pSizeOrFOV, pNearPlane, pFarPlane);
				break;
			case Imgn::CameraType::Orthographic:
				camera.CreateOrthographicCamera(pSizeOrFOV, pNearPlane, pFarPlane);
				break;
			}
		}

		void Serialize(std::fstream& pStream) override;
		void Deserialize(std::fstream& pStream) override;
	};
}