struct VSOutput
{
    float4 pos : SV_Position;
    float3 nrm : NORMAL0;
    float2 uv : TEXCOORD0;
    float3 tan : TANGENT;
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
    output.Position = input.pos;
    
    float3 N = normalize(input.nrm);
    float3 T = normalize(input.tan);
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);
  // Use a flat tangent-space normal (0, 0, 1)
    float3 flatNormal = float3(0.0, 0.0, 1.0);

    // Transform flat tangent-space normal to world space
    float3 worldNormal = normalize(mul(flatNormal, TBN));

    // Output world-space normal
    output.Normal = float4(worldNormal, 1.0);
    output.UV = float4(0.7, 0.7, 0.7, 1);
    return output;
}