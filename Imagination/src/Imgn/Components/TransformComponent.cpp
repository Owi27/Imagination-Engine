#include "pch.hpp"
#include "TransformComponent.h"

namespace Imgn
{
	void TransformComponent::Serialize(std::fstream& pStream)
	{
		pStream.write(reinterpret_cast<const char*>(&TypeID), sizeof(ID));
		pStream.write(reinterpret_cast<const char*>(position.data()), position.size() * sizeof(float));

		vec3 eulerRotation = Math::EulerFromQuat(rotation);
		pStream.write(reinterpret_cast<const char*>(eulerRotation.data()), eulerRotation.size() * sizeof(float));

		pStream.write(reinterpret_cast<const char*>(scale.data()), scale.size() * sizeof(float));
	}

	void TransformComponent::Deserialize(std::fstream& pStream)
	{
		vec3 eulerRotation;

		pStream.read(reinterpret_cast<char*>(position.data()), position.size() * sizeof(float));
		pStream.read(reinterpret_cast<char*>(eulerRotation.data()), eulerRotation.size() * sizeof(float));
		pStream.read(reinterpret_cast<char*>(scale.data()), scale.size() * sizeof(float));

		rotation = Math::QuatFromEuler(eulerRotation);
	}
}