#include "pch.hpp"
#include "SceneSerializer.h"
#include "ImgnComponent.h"
#include "Components/ImgnComponents.h"

namespace Imgn
{
	struct SceneFileHeader
	{
		std::array<char, 4> magic = { 'I', 'M', 'G', 'N' };
		uint32_t version = 1;
		uint32_t entityCount = 0;
	};

	struct MyStruct
	{

	};

	void SceneSerializer::Serialize(const std::filesystem::path& pFilepath)
	{
		if (pFilepath.has_parent_path())
		{
			std::filesystem::create_directories(pFilepath.parent_path());
		}

		_stream.open(pFilepath, std::ios::binary | std::ios::out | std::ios::trunc);

		SceneFileHeader fileHeader
		{
			.entityCount = static_cast<uint32_t>(_scene->GetEntities().size())
		};

		std::string entityHeader = "Entities";
		std::string entityKey = "Entity";

		Write(&fileHeader, sizeof(SceneFileHeader));
		Write(entityHeader.data(), entityHeader.size());

		for (auto& entity : _scene->GetEntities())
		{
			if (!entity) return;

			//serialize entity
			ID id = entity->GetID();
			Write(&id, sizeof(id));
			uint64_t componentCount = entity->GetComponents().size();
			Write(&componentCount, sizeof(componentCount));

			for (auto& comp : entity->GetComponents())
			{
				comp->Serialize(_stream);
			}
		}

		_stream.flush();
		_stream.close();
	}
	void SceneSerializer::SerializeRuntime(const std::string& pFilepath)
	{
	}
	bool SceneSerializer::Deserialize(const std::string& pFilepath)
	{
		_stream.open(pFilepath, std::ios::binary | std::ios::out | std::ios::trunc);

		Scene test;

		SceneFileHeader fileHeader;
		Read(&fileHeader, sizeof(SceneFileHeader));

		if (fileHeader.magic == std::array{ 'I', 'M', 'G', 'N' })
		{
			constexpr uint64_t ENTITY_HEADER_SIZE = 8;
			std::string entityHeader(ENTITY_HEADER_SIZE, '\0');
			Read(entityHeader.data(), ENTITY_HEADER_SIZE);

			for (uint32_t i = 0; i < fileHeader.entityCount; i++)
			{
				ID entityID = 0;
				Read(&entityID, sizeof(entityID));

				uint64_t componentCount = 0;
				Read(&componentCount, sizeof(componentCount));

				if (Entity* entity = _scene->CreateEntity())
				{
					for (uint64_t i = 0; i < componentCount; i++)
					{
						ID componentID = 0;
						Read(&componentID, sizeof(ID));
					}
				}

			}

			_stream.close();

			return true;
		}

		return false;
	}
	bool SceneSerializer::DeserializeRuntime(const std::string& pFilepath)
	{
		return false;
	}
}
