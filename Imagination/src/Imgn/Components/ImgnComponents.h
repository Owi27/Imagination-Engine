#pragma once
#include "Imgn/ImgnComponent.h"
#include "Imgn/ImgnMath.h"
#include "Imgn/ImgnCamera.h"

namespace Imgn
{
	struct TransformComponent : public Component
	{
		IMGN_COMPONENT_ID("Imgn.TransformComponent");

		vec3 position = { 0.f, 0.f, 0.f }, rotation = { 0.f, 0.f, 0.f }, scale = { 1.f, 1.f, 1.f };

		// Previous-frame world matrix for object motion vectors (runtime only; not serialized).
		mat4 prevTransform = Math::identity;
		bool prevTransformValid = false;

		mat4 GetTransform()
		{
			mat4 transform = Math::identity;
			transform = Math::Translate(transform, position);
			transform = Math::Rotate(transform, { 1.f, 0.f, 0.f }, Math::Radians(rotation[0]));
			transform = Math::Rotate(transform, { 0.f, 1.f, 0.f }, Math::Radians(rotation[1]));
			transform = Math::Rotate(transform, { 0.f, 0.f, 1.f }, Math::Radians(rotation[2]));
			transform = Math::Scale(transform, scale);

			return transform;
		}

		TransformComponent() : Component("Transform") {}

		void Serialize(std::fstream& pStream) override
		{
			pStream.write(reinterpret_cast\u003cconst char*\u003e(&TypeID), sizeof(ID));
			pStream.write(reinterpret_cast\u003cconst char*\u003e(position.data()), position.size() * sizeof(float));
			pStream.write(reinterpret_cast\u003cconst char*\u003e(rotation.data()), rotation.size() * sizeof(float));
			pStream.write(reinterpret_cast\u003cconst char*\u003e(scale.data()), scale.size() * sizeof(float));
		}

		void Deserialize(std::fstream& pStream) override
		{
			pStream.read(reinterpret_cast\u003cchar*\u003e(position.data()), position.size() * sizeof(float));
			pStream.read(reinterpret_cast\u003cchar*\u003e(rotation.data()), rotation.size() * sizeof(float));
			pStream.read(reinterpret_cast\u003cchar*\u003e(scale.data()), scale.size() * sizeof(float));
		}
	};

	struct MeshComponent : public Component
	{
		IMGN_COMPONENT_ID("Imgn.MeshComponent");

		uint32_t mesh;
		std::vector\u003cuint32_t\u003e materials;

		bool visible = true;

		MeshComponent() : Component("Mesh") {}
		MeshComponent(uint32_t pMesh) : Component("Mesh")
		{
			mesh = pMesh;
		}

		void Serialize(std::fstream& pStream) override
		{
			pStream.write(reinterpret_cast\u003cconst char*\u003e(&TypeID), sizeof(ID));
		}

		void Deserialize(std::fstream& pStream) override
		{
		}
	};

	struct CameraComponent : public Component
	{
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

		void Serialize(std::fstream& pStream) override
		{
			pStream.write(reinterpret_cast\u003cconst char*\u003e(&TypeID), sizeof(ID));
			uint8_t camType = static_cast\u003cuint8_t\u003e(camera.GetType());
			pStream.write(reinterpret_cast\u003cconst char*\u003e(&camType), sizeof(CameraType));
			pStream.write(reinterpret_cast\u003cconst char*\u003e(&camera.GetFOV()), sizeof(float));
			pStream.write(reinterpret_cast\u003cconst char*\u003e(&camera.GetNearPlane()), sizeof(float));
			pStream.write(reinterpret_cast\u003cconst char*\u003e(&camera.GetFarPlane()), sizeof(float));
			pStream.write(reinterpret_cast\u003cconst char*\u003e(&camera.GetOrthoSize()), sizeof(float));
			pStream.write(reinterpret_cast\u003cconst char*\u003e(&camera.GetOrthoNear()), sizeof(float));
			pStream.write(reinterpret_cast\u003cconst char*\u003e(&camera.GetOrthoFar()), sizeof(float));
			pStream.write(reinterpret_cast\u003cconst char*\u003e(&mainCamera), sizeof(bool));
			pStream.write(reinterpret_cast\u003cconst char*\u003e(&fixedAspect), sizeof(bool));
		}

		void Deserialize(std::fstream& pStream) override
		{
			float fov, pNear, pFar, oSize, oNear, oFar;
			uint8_t camType;

			pStream.read(reinterpret_cast\u003cchar*\u003e(&camType), sizeof(CameraType));
			pStream.read(reinterpret_cast\u003cchar*\u003e(&fov), sizeof(float));
			pStream.read(reinterpret_cast\u003cchar*\u003e(&pNear), sizeof(float));
			pStream.read(reinterpret_cast\u003cchar*\u003e(&pFar), sizeof(float));
			pStream.read(reinterpret_cast\u003cchar*\u003e(&oSize), sizeof(float));
			pStream.read(reinterpret_cast\u003cchar*\u003e(&oNear), sizeof(float));
			pStream.read(reinterpret_cast\u003cchar*\u003e(&oFar), sizeof(float));
			pStream.read(reinterpret_cast\u003cchar*\u003e(&mainCamera), sizeof(bool));
			pStream.read(reinterpret_cast\u003cchar*\u003e(&fixedAspect), sizeof(bool));

			switch (static_cast\u003cCameraType\u003e(camType))
			{
			case Imgn::CameraType::Perspective:
				camera.CreatePerspectiveCamera(fov, pNear, pFar);
				break;
			case Imgn::CameraType::Orthographic:
				camera.CreateOrthographicCamera(oSize, oNear, oFar);
				break;
			}
		}
	};

	struct ScriptComponent : public Component
	{
		IMGN_COMPONENT_ID("Imgn.ScriptComponent");

		unique\u003cScriptableEntity\u003e instance = nullptr;

		std::function\u003cvoid()\u003e Create;
		std::function\u003cvoid()\u003e Destroy;
		std::function\u003cvoid()\u003e Sleep;
		std::function\u003cvoid()\u003e WakeUp;
		std::function\u003cvoid(Time pTime)\u003e Dream;

		template\u003ctypename T\u003e
			requires std::derived_from\u003cT, ScriptableEntity\u003e
		void Bind()
		{
			Create = [&]() { instance = Unique\u003cT\u003e(); };
			Sleep = [&]() { static_cast\u003cT*\u003e(instance.get())->Sleep(); };
			WakeUp = [&]() { static_cast\u003cT*\u003e(instance.get())->WakeUp(); };
			Dream = [&](Time pTime) { static_cast\u003cT*\u003e(instance.get())->Dream(pTime); };
		}

		void Serialize(std::fstream& pStream) override
		{
			pStream.write(reinterpret_cast\u003cconst char*\u003e(&TypeID), sizeof(ID));
		}

		void Deserialize(std::fstream& pStream) override
		{
		}
	};
}