struct VSOutput
{
    float4 pos : SV_Position;
    float3 nrm : NORMAL0;
    float2 uv : TEXCOORD0;
};

struct FSOutput
{
    float4 Position : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 UV : SV_Target2;
};

FSOutput main(VSOutput input)
{
    FSOutput output;
    output.Position = normalize(input.pos);
    output.Normal = float4(input.nrm, 1);
    output.UV = float4(input.uv.x, 0, input.uv.y, 1);
    return output;
}