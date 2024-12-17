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
    float4 prev : TEXCOORD1;
    float4 curr : TEXCOORD2;
};

cbuffer UniformBuffer : register(b0)
{
    matrix world, view, proj;
    float deltaTime;
    matrix prev, curr;
};
   

VSOutput main(VSInput input)
{
    VSOutput output;
    output.pos = mul(proj, mul(view, mul(world, float4(input.pos, 1))));
    output.nrm = -normalize(input.nrm);
    output.uv = input.uv;
    output.curr = mul(curr, float4(input.pos, 1));
    output.prev = mul(prev, float4(input.pos, 1));
    return output;
}