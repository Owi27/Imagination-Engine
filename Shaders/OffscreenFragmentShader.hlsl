struct VSOutput
{
    float4 pos : SV_Position;
    float3 nrm : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 prev : TEXCOORD1;
    float4 curr : TEXCOORD2;
};

struct FSOutput
{
    float4 Position : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 UV : SV_Target2;
    float4 Velocity : SV_Target3;
};

FSOutput main(VSOutput input)
{
    FSOutput output;
    output.Position = normalize(input.pos);
    output.Normal = float4(input.nrm, 1);
    output.UV = float4(0.7, 0.7, 0.7, 1);
    
    float3 c = input.curr.xyz / input.curr.w, p = input.prev.xyz / input.prev.w, v = (c - p) / 0.1;
    output.Velocity = float4(v, 1);
    return output;
}