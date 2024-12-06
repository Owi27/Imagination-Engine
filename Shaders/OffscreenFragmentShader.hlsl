struct VSOutput
{
    float4 pos : SV_Position;
    float3 nrm : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 prevPos : TEXCOORD1;
};

struct FSOutput
{
    float4 Position : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 UV : SV_Target2;
};

cbuffer UniformBuffer : register(b0)
{
    matrix prev, curr;
    float deltaTime;
};

FSOutput main(VSOutput input)
{
    FSOutput output;
    output.Position = normalize(input.pos);
    output.Normal = float4(input.nrm, 1);
    
    float3 curr = input.pos.xyz / input.pos.w;
    float3 prev = input.prevPos.xyz / input.prevPos.w;
    float3 velocity = (curr - prev) / deltaTime;
    
    output.UV = float4(velocity, 1);
    return output;
}