struct VSInput
{
    float3 pos : POSITION0;
    float3 nrm : NORMAL0;
    float2 uv : TEXCOORD0;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 nrm : NORMAL0;
    float2 uv : TEXCOORD0;
};

cbuffer UniformBuffer : register(b0)
{
    matrix world, view, proj;
};
   

VSOutput main(VSInput input)
{
    VSOutput output;
    output.pos = mul(proj, mul(view, mul(world, float4(input.pos, 1))));
    output.nrm = input.nrm;
    output.uv = input.uv;
    return output;
}