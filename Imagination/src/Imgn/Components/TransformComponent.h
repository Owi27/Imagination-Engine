#pragma once
#include "Imgn/ImgnComponent.h"
#include "Imgn/ImgnMath.h"

namespace Imgn
{
	class IMGN_API TransformComponent : public Component
	{
	public:
		IMGN_COMPONENT_ID("Imgn.TransformComponent");

		vec3 position = { 0.f, 0.f, 0.f }, scale = { 1.f, 1.f, 1.f };
		quat rotation = { 0.f, 0.f, 0.f, 1.f };

		mat4 prevTransform;

		mat4 GetTransform()
		{
			mat4 transform = Math::identity;
			transform = Math::Translate(transform, position);
			transform = Math::Rotate(transform, rotation);
			transform = Math::Scale(transform, scale);

			return transform;
		}

		TransformComponent() : Component("Transform") {}

		// Inherited via Component
		void Serialize(std::fstream& pStream) override;
		void Deserialize(std::fstream& pStream) override;
	};
}