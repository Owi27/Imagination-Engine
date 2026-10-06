struct VIn
{
    float3 pos : POSITION0;
};

struct VOut
{
    float4 pos : SV_Position;
    float3 wPos : POSITION0;
};

[[vk::push_constant]]
struct ShadowPC
{
    matrix model;
} pc;

struct ShadowUBO
{
    matrix viewProj;

    float3 lightPosition;
    float farPlane;
};

ConstantBuffer<ShadowUBO> shadow : register(b0, space0);

VOut main(VIn input)
{
    VOut output;

    float4 worldPosition = mul(pc.model, float4(input.pos, 1.f));

    output.pos = mul(shadow.viewProj, worldPosition);
    output.wPos = worldPosition.xyz;

    return output;
}