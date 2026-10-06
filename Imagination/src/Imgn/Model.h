#pragma once
#include "Component/MeshComponent.h"
#include "Component/MaterialComponent.h"

namespace Imgn
{
    class IMGN_API Model
    {
        

    public:
        Model() /*Constructor*/
        {
        }

        ~Model() /*Destructor*/
        {
        }

        /*Copy Constructor*/
        Model(const Model& pOther) = default;

        /*Copy Assignment Operator*/
        Model& operator=(const Model& pOther) = default;

        /*Move Constructor*/
        Model(Model&& pOther) noexcept = default;

        /*Move Assignment Operator*/
        Model& operator=(Model&& pOther) noexcept = default;

        /*Class Functions*/
    };
}