#include "pch.hpp"
#include "CameraComponent.h"

namespace Imgn
{
	void CameraComponent::Serialize(std::fstream& pStream)
	{
		pStream.write(reinterpret_cast<const char*>(&TypeID), sizeof(ID));

		uint8_t cameraType = static_cast<uint8_t>(camera.GetType());

		pStream.write(reinterpret_cast<const char*>(&cameraType), sizeof(uint8_t));

		pStream.write(reinterpret_cast<const char*>(&camera.GetFOV()), sizeof(float));
		pStream.write(reinterpret_cast<const char*>(&camera.GetAspect()), sizeof(float));
		pStream.write(reinterpret_cast<const char*>(&camera.GetNearPlane()), sizeof(float));
		pStream.write(reinterpret_cast<const char*>(&camera.GetFarPlane()), sizeof(float));

		pStream.write(reinterpret_cast<const char*>(&camera.GetOrthoSize()), sizeof(float));
		pStream.write(reinterpret_cast<const char*>(&camera.GetOrthoNear()), sizeof(float));
		pStream.write(reinterpret_cast<const char*>(&camera.GetOrthoFar()), sizeof(float));

		pStream.write(reinterpret_cast<const char*>(&mainCamera), sizeof(bool));
		pStream.write(reinterpret_cast<const char*>(&fixedAspect), sizeof(bool));
		pStream.write(reinterpret_cast<const char*>(&cameraSpeed), sizeof(float));
	}

	void CameraComponent::Deserialize(std::fstream& pStream)
	{
		uint8_t cameraType = 0;

		float fov = Math::Radians(45.f);
		float aspect = 16.f / 9.f;
		float nearPlane = .1f;
		float farPlane = 5000.f;

		float orthoSize = 1.f;
		float orthoNear = -1.f;
		float orthoFar = 1.f;

		pStream.read(reinterpret_cast<char*>(&cameraType), sizeof(uint8_t));

		pStream.read(reinterpret_cast<char*>(&fov), sizeof(float));
		pStream.read(reinterpret_cast<char*>(&aspect), sizeof(float));
		pStream.read(reinterpret_cast<char*>(&nearPlane), sizeof(float));
		pStream.read(reinterpret_cast<char*>(&farPlane), sizeof(float));

		pStream.read(reinterpret_cast<char*>(&orthoSize), sizeof(float));
		pStream.read(reinterpret_cast<char*>(&orthoNear), sizeof(float));
		pStream.read(reinterpret_cast<char*>(&orthoFar), sizeof(float));

		pStream.read(reinterpret_cast<char*>(&mainCamera), sizeof(bool));
		pStream.read(reinterpret_cast<char*>(&fixedAspect), sizeof(bool));
		pStream.read(reinterpret_cast<char*>(&cameraSpeed), sizeof(float));

		camera.SetType(static_cast<CameraType>(cameraType));

		camera.GetFOV() = fov;
		camera.GetAspect() = aspect;
		camera.GetNearPlane() = nearPlane;
		camera.GetFarPlane() = farPlane;

		camera.GetOrthoSize() = orthoSize;
		camera.GetOrthoNear() = orthoNear;
		camera.GetOrthoFar() = orthoFar;

		camera.RecalculateProjection();
	}
}