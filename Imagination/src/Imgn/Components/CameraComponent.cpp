#include "pch.hpp"
#include "CameraComponent.h"

namespace Imgn
{
	void CameraComponent::Serialize(std::fstream& pStream)
	{
		pStream.write(reinterpret_cast<const char*>(&TypeID), sizeof(ID));
		uint8_t camType = static_cast<uint8_t>(camera.GetType());
		pStream.write(reinterpret_cast<const char*>(&camType), sizeof(CameraType));
		pStream.write(reinterpret_cast<const char*>(&camera.GetFOV()), sizeof(float));
		pStream.write(reinterpret_cast<const char*>(&camera.GetNearPlane()), sizeof(float));
		pStream.write(reinterpret_cast<const char*>(&camera.GetFarPlane()), sizeof(float));
		pStream.write(reinterpret_cast<const char*>(&camera.GetOrthoSize()), sizeof(float));
		pStream.write(reinterpret_cast<const char*>(&camera.GetOrthoNear()), sizeof(float));
		pStream.write(reinterpret_cast<const char*>(&camera.GetOrthoFar()), sizeof(float));
		pStream.write(reinterpret_cast<const char*>(&mainCamera), sizeof(bool));
		pStream.write(reinterpret_cast<const char*>(&fixedAspect), sizeof(bool));
	}

	void CameraComponent::Deserialize(std::fstream& pStream)
	{
		float fov, pNear, pFar, oSize, oNear, oFar;
		uint8_t camType;

		pStream.read(reinterpret_cast<char*>(&camType), sizeof(CameraType));
		pStream.read(reinterpret_cast<char*>(&fov), sizeof(float));
		pStream.read(reinterpret_cast<char*>(&pNear), sizeof(float));
		pStream.read(reinterpret_cast<char*>(&pFar), sizeof(float));
		pStream.read(reinterpret_cast<char*>(&oSize), sizeof(float));
		pStream.read(reinterpret_cast<char*>(&oNear), sizeof(float));
		pStream.read(reinterpret_cast<char*>(&oFar), sizeof(float));
		pStream.read(reinterpret_cast<char*>(&mainCamera), sizeof(bool));
		pStream.read(reinterpret_cast<char*>(&fixedAspect), sizeof(bool));

		switch (static_cast<CameraType>(camType))
		{
		case Imgn::CameraType::Perspective:
			camera.CreatePerspectiveCamera(fov, pNear, pFar);
			break;
		case Imgn::CameraType::Orthographic:
			camera.CreateOrthographicCamera(oSize, oNear, oFar);
			break;
		}
	}
}