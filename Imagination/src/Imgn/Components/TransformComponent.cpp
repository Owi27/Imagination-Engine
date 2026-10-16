#include "pch.hpp"
#include "TransformComponent.h"

namespace Imgn
{
	void TransformComponent::Serialize(std::fstream& pStream)
	{
		pStream.write(reinterpret_cast<const char*>(&TypeID), sizeof(ID));

		pStream.write(reinterpret_cast<const char*>(position.data()), position.size() * sizeof(float));
		pStream.write(reinterpret_cast<const char*>(rotation.data()), rotation.size() * sizeof(float));
		pStream.write(reinterpret_cast<const char*>(scale.data()), scale.size() * sizeof(float));
	}

	void TransformComponent::Deserialize(std::fstream& pStream)
	{
		pStream.read(reinterpret_cast<char*>(position.data()), position.size() * sizeof(float));
		pStream.read(reinterpret_cast<char*>(rotation.data()), rotation.size() * sizeof(float));
		pStream.read(reinterpret_cast<char*>(scale.data()), scale.size() * sizeof(float));

		prevTransform = GetTransform();
	}
}