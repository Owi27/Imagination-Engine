struct VOut
{
    float4 pos : SV_Position;
    float3 wPos : POSITION;
};

struct FOut
{
    float4 PointShadowArray : SV_TARGET0;
};

struct ShadowUBO
{
    matrix viewProj;

    float3 lightPosition;
    float farPlane;
};

ConstantBuffer<ShadowUBO> shadow : register(b0, space0);

struct PointLight
{
    float3 pos, col;
    float range, intensity;
};

StructuredBuffer<PointLight> pointLights : register(t1, space0);

Texture2D materialTextures[] : register(t0, space1);
SamplerState materialSampler : register(s0, space1);

float2 ClipToUV(float4 pClip)
{
    float2 ndc = pClip.xy / pClip.w;
    
    return ndc * .5f + .5f;
}

float main(VOut input) : SV_Depth
{
    float distanceToLight = length(input.wPos - shadow.lightPosition);
    float normalizedDepth = saturate(distanceToLight / shadow.farPlane);

    return 1.f - normalizedDepth;
}