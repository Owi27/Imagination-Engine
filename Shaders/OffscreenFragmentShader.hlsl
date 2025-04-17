Texture2D materialTexures[] : register(t0, space1);
SamplerState materialSampler : register(s0, space1);

struct Material
{
    float4 baseColorFactor;
    int baseColorTexture;
    float metallicFactor;
    float roughnessFactor;
    int metallicRoughnessTexture;
    int emissiveTexture;
    float3 emissiveFactor;
    int alphaMode;
    float alphaCutoff;
    int doubleSided;
    int normalTexture;
    float normalTextureScale;
    int occlusionTexture;
    float occlusionTextureStrength;
};

StructuredBuffer<Material> materialInfo : register(t1, space0);

struct VSOutput
{
    float4 pos : SV_Position;
    float3 nrm : NORMAL0;
    float2 uv : TEXCOORD0;
    float3 tan : TANGENT;
    nointerpolation int idx : INDEX;
};

struct FSOutput
{
    float4 Position : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 albedo : SV_TARGET2;
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
    
    if (materialInfo[input.idx].baseColorTexture > -1) 
        output.albedo = materialTexures[materialInfo[input.idx].baseColorTexture].Sample(materialSampler, input.uv);
    else
        output.albedo = materialInfo[input.idx].baseColorFactor;
    
 //   output.albedo = materialTexures[0].Sample(materialSamplers[0], input.uv);
    //materialInfo[input.idx].b
   // output.UV = float4(0.7, 0.7, 0.7, 1);
    return output;
}