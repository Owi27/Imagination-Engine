struct VSInput
{
    float3 pos : POSITION0;
    float3 nrm : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 tan : TANGENT;
    //nointerpolation int matId : SV_InstanceID;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 nrm : NORMAL0;
    float2 uv : TEXCOORD0;
    float3 tan : TANGENT;
    float3x3 TBN : TEXCOORD1;
    int idx : INDEX;
};

cbuffer UniformBuffer : register(b0)
{
    matrix world, view, proj;
    float deltaTime;
};

struct PCR
{
    matrix model, normal;
};

[[vk::push_constant]] PCR _pcr;
   

VSOutput main(VSInput input, uint id : SV_InstanceID)
{
    VSOutput output;
    output.pos = mul(proj, mul(view, mul(mul(world, _pcr.model), float4(input.pos, 1))));
    output.uv = input.uv;
    
   //transform normal and tangent to world space
    float3x3 worldMatrix3x3 = (float3x3) (mul(world, _pcr.model));

// If non-uniform scale is used, use inverse transpose
    float3x3 normalMatrix = (float3x3)_pcr.normal;
    
    //output.nrm = normalize(mul(normalMatrix, input.nrm));
    //output.tan = normalize(mul(worldMatrix3x3, input.tan.xyz));
    
    output.nrm = normalize(mul(_pcr.model, float4(input.nrm, 0))).xyz;
    output.tan = normalize(mul(_pcr.model, float4(input.tan.xyz, 0))).xyz;
    output.TBN = float3x3(output.tan, cross(output.nrm, output.tan), output.nrm);
    output.idx = id;
    
    return output;
}