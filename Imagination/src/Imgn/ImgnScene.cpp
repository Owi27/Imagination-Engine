#include "pch.hpp"
#include "ImgnScene.h"
#include "Components/AllComponents.hpp"
#include "ImgnGLTF.h"

#include <unordered_map>

namespace Imgn
{
	ScriptComponent* Scene::GetOrCreateScript(Entity* pEntity)
	{
		if (!pEntity || !pEntity->IsActive()) return nullptr;

		ScriptComponent* script = pEntity->GetComponent<ScriptComponent>();

		if (!script || !script->IsInitialized()) return nullptr;
		if (script->instance) return script;

		if (!script->Create || !script->Sleep || !script->WakeUp || !script->Dream)
		{
			IMGN_CORE_ERROR("Script on entity '{}' has no complete binding.", pEntity->GetName());
			return nullptr;
		}

		script->Create();

		if (!script->instance)
		{
			IMGN_CORE_ERROR("Script creation failed for entity '{}'.", pEntity->GetName());
			return nullptr;
		}

		script->instance->_entity = pEntity;
		script->Sleep();

		return script;
	}

	void Scene::Dream(Time pTime)
	{
		for (const unique<Entity>& entity : _entities)
		{
			entity->Dream(pTime.Seconds());
			if (ScriptComponent* script = GetOrCreateScript(entity.get())) script->Dream(pTime);
		}
	}

	Entity* Scene::CreateEntity(const std::string& pName, ID pID)
	{
		if (pID != 0 && GetEntity(pID))
		{
			IMGN_CORE_ERROR("Cannot create entity '{}' with duplicate ID {}.", pName, pID);
			return nullptr;
		}

		_entities.emplace_back(Unique<Entity>(pName, pID));

		Entity* entity = _entities.back().get();
		entity->AddComponent<TransformComponent>();

		return entity;
	}

	shared<Scene> Scene::Clone() const
	{
		shared<Scene> scene = Shared<Scene>();
		scene->_viewportWidth = _viewportWidth;
		scene->_viewportHeight = _viewportHeight;
		scene->_entities.reserve(_entities.size());

		std::unordered_map<const Entity*, Entity*> entityCopies;
		std::unordered_map<const Material*, shared<Material>> materialCopies;
		entityCopies.reserve(_entities.size());

		for (const unique<Entity>& source : _entities)
		{
			Entity* target = scene->CreateEntity(source->GetName(), source->GetID());
			if (!target) return nullptr;

			entityCopies.emplace(source.get(), target);
			target->SetActive(source->IsActive());

			if (const TransformComponent* component = source->GetComponent<TransformComponent>())
			{
				TransformComponent* copy = target->GetComponent<TransformComponent>();
				copy->position = component->position;
				copy->rotation = component->rotation;
				copy->scale = component->scale;
				copy->prevTransform = copy->GetTransform();
			}
			else target->RemoveComponent<TransformComponent>();

			if (const CameraComponent* component = source->GetComponent<CameraComponent>())
			{
				CameraComponent* copy = target->AddComponent<CameraComponent>();
				copy->camera = component->camera;
				copy->mainCamera = component->mainCamera;
				copy->fixedAspect = component->fixedAspect;
				copy->cameraSpeed = component->cameraSpeed;
			}

			if (const MeshComponent* component = source->GetComponent<MeshComponent>()) target->AddComponent<MeshComponent>()->CopyFrom(*component);

			if (const MaterialComponent* component = source->GetComponent<MaterialComponent>())
			{
				std::vector<shared<Material>> materials;
				materials.reserve(component->GetMaterialCount());

				for (const shared<Material>& material : component->GetMaterials())
				{
					if (!material)
					{
						materials.push_back(nullptr);
						continue;
					}

					auto [it, inserted] = materialCopies.try_emplace(material.get(), nullptr);
					if (inserted) it->second = Shared<Material>(*material);

					materials.push_back(it->second);
				}

				target->AddComponent<MaterialComponent>()->SetMaterials(std::move(materials));
			}

			if (const PointLightComponent* component = source->GetComponent<PointLightComponent>())
			{
				PointLightComponent* copy = target->AddComponent<PointLightComponent>();
				copy->col = component->col;
				copy->range = component->range;
				copy->intensity = component->intensity;
			}

			if (const ScriptComponent* component = source->GetComponent<ScriptComponent>())
			{
				if (!target->AddComponent<ScriptComponent>()->CopyBindingFrom(*component))
				{
					IMGN_CORE_ERROR("Cannot clone script on entity '{}'. Bind its script type first.", source->GetName());
					return nullptr;
				}
			}

			if (target->GetComponents().size() != source->GetComponents().size())
			{
				IMGN_CORE_ERROR("Scene cloning does not support every component on entity '{}'.", source->GetName());
				return nullptr;
			}
		}

		for (const unique<Entity>& source : _entities)
		{
			Entity* parentCopy = entityCopies.at(source.get());

			for (Entity* child : source->GetChildren())
			{
				auto it = entityCopies.find(child);

				if (it == entityCopies.end() || !parentCopy->AddChild(it->second))
				{
					IMGN_CORE_ERROR("Cannot clone hierarchy for entity '{}'.", source->GetName());
					return nullptr;
				}
			}
		}

		return scene;
	}

	void Scene::DestroyEntity(Entity* pEntity)
	{
		if (!pEntity) return;

		pEntity->RemoveFromParent();

		std::vector<Entity*> children = pEntity->GetChildren();

		for (Entity* child : children) pEntity->RemoveChild(child);

		std::erase_if(_entities, [pEntity](const unique<Entity>& pEntityEntry) { return pEntityEntry.get() == pEntity; });
	}

	Entity* Scene::GetEntity(ID pID)
	{
		if (pID == 0) return nullptr;

		for (const unique<Entity>& entity : _entities)
		{
			if (entity->GetID() == pID) return entity.get();
		}

		return nullptr;
	}

	Entity* Scene::GetPrimaryCameraEntity()
	{
		for (auto& entity : _entities)
		{
			if (CameraComponent* cameraComponent = entity->GetComponent<CameraComponent>())
			{
				if (cameraComponent->mainCamera)
				{
					return entity.get();
				}
			}
		}

		return nullptr;
	}

	void Scene::OnViewportResize(uint32_t pWidth, uint32_t pHeight)
	{
		_viewportWidth = pWidth;
		_viewportHeight = pHeight;

		for (auto& entity : _entities)
			if (CameraComponent* cameraComp = entity->GetComponent<CameraComponent>())
				if (!cameraComp->fixedAspect) cameraComp->camera.SetViewportSize(pWidth, pHeight);
	}

	void Scene::RuntimeStart()
	{
		for (const unique<Entity>& entity : _entities)
		{
			entity->Init();
			GetOrCreateScript(entity.get());
		}
	}

	void Scene::RuntimeStop()
	{
		for (const unique<Entity>& entity : _entities)
		{
			ScriptComponent* script = entity->GetComponent<ScriptComponent>();
			if (!script || !script->instance) continue;

			if (script->WakeUp) script->WakeUp();

			script->instance.reset();
		}
	}

	void Scene::LoadMeshAssets(ImgnRenderer& pRenderer)
	{
		std::unordered_map<std::filesystem::path, std::vector<MeshData>> loadedAssets;

		for (const unique<Entity>& entity : GetEntitiesWithComponent<MeshComponent>())
		{
			MeshComponent* mesh = entity->GetComponent<MeshComponent>();
			if (mesh->GetMeshAssetPath().empty()) continue;
			if (mesh->GetVertexBuffer() && mesh->GetIndexBuffer() && !mesh->GetPrimitives().empty()) continue;

			const std::filesystem::path assetPath = mesh->GetMeshAssetPath().lexically_normal();
			loadedAssets[assetPath] = GLTFLoader::LoadMeshes(assetPath, pRenderer);
			
			if (loadedAssets[assetPath].empty()) continue;
			
			const MeshData* source = nullptr;

			for (const MeshData& candidate : loadedAssets[assetPath])
			{
				if (candidate.name != mesh->GetMeshName()) continue;
				if (source)
				{
					source = nullptr;
					break;
				}

				source = &candidate;
			}

			if (!source)
			{
				IMGN_CORE_ERROR("Cannot restore mesh '{}' from '{}': the name is missing or ambiguous.", mesh->GetMeshName(), assetPath.string());
				continue;
			}

			mesh->SetMesh(mesh->GetMeshName(), mesh->GetMeshAssetPath(), source->vertexBuffer, source->indexBuffer, source->primitives);
		}
	}
}