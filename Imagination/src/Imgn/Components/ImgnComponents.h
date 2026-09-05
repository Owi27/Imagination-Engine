#pragma once
#include "Imgn/ImgnComponent.h"
#include "Imgn/ImgnMath.h"
#include "Imgn/ImgnCamera.h"

namespace Imgn
{
	struct TransformComponent : public Component
	{
		//static constexpr ID TypeID = HashComponentName("Imgn.TransformComponent");
		IMGN_COMPONENT_ID("Imgn.TransformComponent");

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

		TransformComponent() : Component("Transform") {}

		// Inherited via Component
		void Serialize(std::fstream& pStream) override
		{
			pStream.write(reinterpret_cast<const char*>(&TypeID), sizeof(ID));
			pStream.write(reinterpret_cast<const char*>(position.data()), position.size() * sizeof(float));
			pStream.write(reinterpret_cast<const char*>(rotation.data()), rotation.size() * sizeof(float));
			pStream.write(reinterpret_cast<const char*>(scale.data()), scale.size() * sizeof(float));
		}

		// Inherited via Component
		void Deserialize(std::fstream& pStream) override
		{
			pStream.read(reinterpret_cast<char*>(position.data()), position.size() * sizeof(float));
			pStream.read(reinterpret_cast<char*>(rotation.data()), rotation.size() * sizeof(float));
			pStream.read(reinterpret_cast<char*>(scale.data()), scale.size() * sizeof(float));
		}
	};

	struct MeshComponent : public Component
	{
		//static constexpr ComponentTypeID TypeID = HashComponentName("Imgn.MeshComponent");
		IMGN_COMPONENT_ID("Imgn.MeshComponent");

		uint32_t mesh;

		MeshComponent() : Component("Mesh") {}
		MeshComponent(uint32_t pMesh) : Component("Mesh")
		{
			mesh = pMesh;
		}

		void Serialize(std::fstream& pStream) override
		{
			pStream.write(reinterpret_cast<const char*>(&TypeID), sizeof(ID));
			//pStream.write(reinterpret_cast<const char*>(position.data()), position.size() * sizeof(float));
			//pStream.write(reinterpret_cast<const char*>(rotation.data()), rotation.size() * sizeof(float));
			//pStream.write(reinterpret_cast<const char*>(scale.data()), scale.size() * sizeof(float));
		}

		void Deserialize(std::fstream& pStream) override
		{
		}
	};

	struct CameraComponent : public Component
	{
		//static constexpr ComponentTypeID TypeID = HashComponentName("Imgn.CameraComponent");
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

		void Deserialize(std::fstream& pStream) override
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
	};

	struct ScriptComponent : public Component
	{
		//static constexpr ComponentTypeID TypeID = HashComponentName("Imgn.ScriptComponent");
		IMGN_COMPONENT_ID("Imgn.ScriptComponent");

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

		void Serialize(std::fstream& pStream) override
		{
			pStream.write(reinterpret_cast<const char*>(&TypeID), sizeof(ID));
		}

		void Deserialize(std::fstream& pStream) override
		{
		}
	};
}