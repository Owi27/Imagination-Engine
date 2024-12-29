struct VSInput
{
    float3 pos : POSITION0;
    float3 nrm : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 tan : TANGENT;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 nrm : NORMAL0;
    float2 uv : TEXCOORD0;
    float3 tan : TANGENT;
};

cbuffer UniformBuffer : register(b0)
{
    matrix world, view, proj, inverse;
    float deltaTime;
};
   

VSOutput main(VSInput input)
{
    VSOutput output;
    output.pos = mul(proj, mul(view, mul(world, float4(input.pos, 1))));
    output.uv = input.uv;
    
    //normal in world space
    output.nrm = normalize(input.nrm);
    output.tan = normalize(input.tan).xyz;
    
    return output;
}