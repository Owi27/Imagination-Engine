struct VertexOUT
{
    float4 pos : SV_POSITION;
    float3 nrm : NORMAL;
    float2 uv : TEXCOORD0;
    float4 tan : TANGENT;
};

struct FragmentOUT
{
    float4 Normal : SV_TARGET0;
    float4 Albedo : SV_TARGET1;
    float4 Emmissive : SV_TARGET2;
    float4 aoRM : SV_TARGET3;
};

float2 OctEncode(float3 n)
{
    n = normalize(n);
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    float2 p = n.xy;
    if (n.z < 0.0)
        p = (1.0 - abs(p.yx)) * (float2(p.x >= 0 ? 1 : -1, p.y >= 0 ? 1 : -1));
    return p * 0.5 + 0.5; // map to [0,1]
}

FragmentOUT main(VertexOUT input)
{
    FragmentOUT output;
    
    output.Normal = float4(OctEncode(input.nrm), 0, 1);
    //float4(input.nrm, 1);
    output.Albedo = float4(1, 0, 1, 1);
    output.Emmissive = float4(0, 1, 1, 1);
    output.aoRM = float4(1, 1, 1, 1);
   
    return output;
}