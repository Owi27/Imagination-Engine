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
    int _pad;
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
    //float4 emissive : SV_TARGET3;
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
    
    //if (materialInfo[input.idx].normalTexture > -1)
    //{
    //    float3 n = materialTexures[materialInfo[input.idx].normalTexture].Sample(materialSampler, input.uv).xyz;
    //    //flatNormal = normalize(n * 2.0f - 1.0f) * materialInfo[input.idx].normalTextureScale;
    //}

    // Transform flat tangent-space normal to world space
    float3 worldNormal = normalize(mul(flatNormal, TBN));

    // Output world-space normal
    output.Normal = float4(worldNormal, materialInfo[input.idx].roughnessFactor);
    
    output.albedo = materialInfo[input.idx].baseColorFactor;
    if (materialInfo[input.idx].baseColorTexture > -1) 
        output.albedo *= materialTexures[materialInfo[input.idx].baseColorTexture].Sample(materialSampler, input.uv);
    
    output.albedo.a = materialInfo[input.idx].metallicFactor;
    
    ////emissive
    //float3 emissive = materialInfo[input.idx].emissiveFactor;
    //if (materialInfo[input.idx].emissiveTexture > -1) 
    //    emissive *= materialTexures[materialInfo[input.idx].emissiveTexture].Sample(materialSampler, input.uv).rgb;
    
    ////ao
    //float ao = 1;
    //if (materialInfo[input.idx].occlusionTexture > -1)
    //    ao = materialTexures[materialInfo[input.idx].occlusionTexture].Sample(materialSampler, input.uv).r * materialInfo[input.idx].occlusionTextureStrength;
    
    //output.emissive = float4(emissive, ao);
    //else
    //    output.albedo = materialInfo[input.idx].baseColorFactor;
    
    return output;
}