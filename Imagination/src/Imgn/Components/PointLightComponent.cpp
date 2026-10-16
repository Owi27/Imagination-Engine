#include "pch.hpp"
#include "PointLightComponent.h"

namespace Imgn
{
	void PointLightComponent::Serialize(std::fstream& pStream)
	{
		pStream.write(reinterpret_cast<const char*>(&TypeID), sizeof(ID));
		pStream.write(reinterpret_cast<const char*>(col.data()), col.size() * sizeof(float));
		pStream.write(reinterpret_cast<const char*>(&range), sizeof(float));
		pStream.write(reinterpret_cast<const char*>(&intensity), sizeof(float));
	}

	void PointLightComponent::Deserialize(std::fstream& pStream)
	{
		pStream.read(reinterpret_cast<char*>(col.data()), col.size() * sizeof(float));
		pStream.read(reinterpret_cast<char*>(&range), sizeof(float));
		pStream.read(reinterpret_cast<char*>(&intensity), sizeof(float));
	}
}