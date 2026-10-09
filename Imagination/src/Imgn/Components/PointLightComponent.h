#pragma once
#include "Imgn/ImgnComponent.h"

namespace Imgn
{
    class IMGN_API PointLightComponent : public Component
    {
        vec3 _pos, _col;
        float _range, _intensity;

    public:
        PointLightComponent() /*Constructor*/
        {
        }

        ~PointLightComponent() /*Destructor*/
        {
        }

        /*Copy Constructor*/
        PointLightComponent(const PointLightComponent& pOther) = default;

        /*Copy Assignment Operator*/
        PointLightComponent& operator=(const PointLightComponent& pOther) = default;

        /*Move Constructor*/
        PointLightComponent(PointLightComponent&& pOther) noexcept = default;

        /*Move Assignment Operator*/
        PointLightComponent& operator=(PointLightComponent&& pOther) noexcept = default;

        /*Class Functions*/
    };
}