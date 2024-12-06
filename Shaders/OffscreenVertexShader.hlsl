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
    float4 prevPos : TEXCOORD1;
};

cbuffer UniformBuffer : register(b0)
{
    matrix prev, curr;
    float deltaTime;
};
   

VSOutput main(VSInput input)
{
    VSOutput output;
    output.pos = mul(curr, float4(input.pos, 1));
    output.prevPos = mul(prev, float4(input.pos, 1));
    output.nrm = normalize(input.nrm);
    output.uv = input.uv;
    return output;
}