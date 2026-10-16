#include "pch.hpp"
#include "ScriptComponent.h"

namespace Imgn
{
	void ScriptComponent::Serialize(std::fstream& pStream)
	{
		pStream.write(reinterpret_cast<const char*>(&TypeID), sizeof(ID));
	}

	void ScriptComponent::Deserialize(std::fstream& pStream)
	{

	}

}