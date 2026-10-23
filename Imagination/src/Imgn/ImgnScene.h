#pragma once
#include "ImgnComponent.h"
#include "ImgnTime.h"

#include <ranges>

namespace Imgn
{
    class ImgnRenderer;
    class ScriptComponent;

    class IMGN_API Scene
    {
        friend class Entity;
        friend class SceneHierarchy;

        std::vector<unique<Entity>> _entities;
        uint32_t _viewportWidth = 0, _viewportHeight = 0;

        ScriptComponent* GetOrCreateScript(Entity* pEntity);

    public:
        Scene() /*Constructor*/
        {
            

            /*for (auto& entity : _entities)
            {
                if (TransformComponent* t = entity->GetComponent<TransformComponent>())
                    t->transform = { 1 };
            }*/
        }

        ~Scene() /*Destructor*/
        {
        }

        /*Copy Constructor*/
        Scene(const Scene& pOther) = delete;

        /*Copy Assignment Operator*/
        Scene& operator=(const Scene& pOther) = delete;

        /*Move Constructor*/
        Scene(Scene&& pOther) noexcept = default;

        /*Move Assignment Operator*/
        Scene& operator=(Scene&& pOther) noexcept = default;

        void Dream(Time pTime);
        [[nodiscard]] shared<Scene> Clone() const;
        void Clear() { _entities.clear(); }

        /*Class Functions*/
        Entity* CreateEntity(const std::string& pName = "Entity", ID pID = 0);
        void DestroyEntity(Entity* pEntity);
        void DestroyEntity(unique<Entity>& pEntity) { DestroyEntity(pEntity.get()); }
        std::vector<unique<Entity>>& GetEntities() { return _entities; }

        Entity* GetEntity(ID pID);

        Entity* GetPrimaryCameraEntity();

        template<typename T>
        auto GetEntitiesWithComponent() { return _entities | std::views::filter([](const unique<Entity>& pEntity) { return pEntity->HasComponent<T>(); }); }

        void OnViewportResize(uint32_t pWidth, uint32_t pHeight);

        void RuntimeStart();
        void RuntimeStop();

        void LoadMeshAssets(ImgnRenderer& pRenderer);
    };
}