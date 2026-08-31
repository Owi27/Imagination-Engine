#pragma once
#include "Imgn/ImgnComponent.h"
#include "Imgn/ImgnMath.h"
#include "Imgn/ImgnCamera.h"

namespace Imgn
{
	struct TransformComponent : public Component
	{
		static constexpr ComponentTypeID TypeID = HashComponentName("Imgn.TransformComponent");

		vec3 position = { 0.f, 0.f, 0.f }, rotation = { 0.f, 0.f, 0.f }, scale = { 1.f, 1.f, 1.f };

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

		TransformComponent() : Component("Transform")
		{
		}
	};

	struct MeshComponent : public Component
	{
		static constexpr ComponentTypeID TypeID = HashComponentName("Imgn.MeshComponent");

		uint32_t mesh;

		explicit MeshComponent(uint32_t pMesh) : Component("Mesh")
		{
			mesh = pMesh;
		}
	};

	struct CameraComponent : public Component
	{
		static constexpr ComponentTypeID TypeID = HashComponentName("Imgn.CameraComponent");

		Camera camera;
		bool mainCamera = true;
		bool fixedAspect = false;
		float cameraSpeed = 250.f;


		CameraComponent(float pSizeOrFOV = Math::Radians(45.f), float pNearPlane = .1f, float pFarPlane = 1000.f, CameraType pType = CameraType::Perspective) : Component("Camera")
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
	};

	struct ScriptComponent : public Component
	{
		static constexpr ComponentTypeID TypeID = HashComponentName("Imgn.ScriptComponent");
		unique<ScriptableEntity> instance = nullptr;

		std::function<void()> Create;
		std::function<void()> Destroy;
		std::function<void()> Sleep;
		std::function<void()> WakeUp;
		std::function<void(Time pTime)> Dream;

		template<typename T>
			requires std::derived_from<T, ScriptableEntity>
		void Bind()
		{
			Create = [&]() { instance = Unique<T>(); };
			Sleep = [&]() { static_cast<T*>(instance.get())->Sleep(); };
			WakeUp = [&]() { static_cast<T*>(instance.get())->WakeUp(); };
			Dream = [&](Time pTime) { static_cast<T*>(instance.get())->Dream(pTime); };
		}
	};
}