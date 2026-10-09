#pragma once
#include "Imgn/ImgnComponent.h"
#include "ImgnComponents.h"

namespace Imgn
{
    struct PointLight
    {
        vec4 posRange, colIntensity;
    };

    class IMGN_API PointLightComponent : public Component
    {

    public:
        IMGN_COMPONENT_ID("Imgn.PointLightComponent");

        vec3 col;
        float range, intensity;

        PointLightComponent() /*Constructor*/ : Component("PointLight")
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
        PointLight GetPointLight(TransformComponent* pTC)
        {
            return PointLight
            {
                .posRange = { pTC->position[0], pTC->position[1], pTC->position[2], range },
                .colIntensity = { col[0], col[1], col[2], intensity }
            };
        }

        // Inherited via Component
        void Serialize(std::fstream& pStream) override;
        void Deserialize(std::fstream& pStream) override;
    };
}