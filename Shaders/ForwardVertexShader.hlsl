cbuffer UniformBuffer : register(b0)
{
    matrix view, proj;
};

struct PCR
{
    matrix model, normal;
};

[[vk::push_constant]] PCR _pcr;

float4 main( float3 pos : POSITION ) : SV_POSITION
{
    return mul(proj, mul(view, mul( _pcr.model, float4(pos, 1))));
}