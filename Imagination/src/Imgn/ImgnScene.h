#pragma once
#include "ImgnComponent.h"
#include "ImgnTime.h"

namespace Imgn
{
    class IMGN_API Scene
    {
        std::vector<unique<Entity>> _entities;
        uint32_t _viewportWidth = 0, _viewportHeight = 0;

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

        /*Class Functions*/
        Entity* CreateEntity(const std::string& pName = "Entity");
        std::vector<unique<Entity>>& GetEntities() { return _entities; }

        void OnViewportResize(uint32_t pWidth, uint32_t pHeight);
    };
}