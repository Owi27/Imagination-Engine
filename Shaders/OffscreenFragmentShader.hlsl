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
    float3x3 TBN : TEXCOORD1;
    nointerpolation int idx : INDEX;
};

struct FSOutput
{
    float4 Position : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 Albedo : SV_TARGET2;
    float4 MProperties : SV_TARGET3;
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
    Material m = materialInfo[input.idx];
        
    float3 worldNormal;
    if (m.normalTexture > -1)
    {
        float3 normalMap = materialTexures[m.normalTexture].Sample(materialSampler, input.uv).xyz;
        float3 tangentNormal = normalize(normalMap * 2.0f - 1.0f);
        worldNormal = normalize(mul(input.TBN, tangentNormal));
    }
    else worldNormal = normalize(N);
    
    // Output world-space normal
    output.Normal = float4(worldNormal, m.roughnessFactor);
    
    output.Albedo = materialInfo[input.idx].baseColorFactor;
    if (materialInfo[input.idx].baseColorTexture > -1) 
        output.Albedo *= materialTexures[materialInfo[input.idx].baseColorTexture].Sample(materialSampler, input.uv);
    
    //output.Albedo.a = materialInfo[input.idx].roughnessFactor;
    
    //emissive
    float3 emissive = m.emissiveFactor;
    if (m.emissiveTexture > -1) 
        emissive *= materialTexures[m.emissiveTexture].Sample(materialSampler, input.uv).rgb;
    
    float emissiveMask = step(0.001, dot(emissive.rgb, float3(1.f, 1.f, 1.f)));
    
    //ao
    float ao = 1;
    if (materialInfo[input.idx].occlusionTexture > -1)
        ao = materialTexures[materialInfo[input.idx].occlusionTexture].Sample(materialSampler, input.uv).r * materialInfo[input.idx].occlusionTextureStrength;
   // output.emissive = float4(emissive, ao);
    //else
    //    output.albedo = materialInfo[input.idx].baseColorFactor;
    
    output.MProperties = float4(m.metallicFactor, emissiveMask, ao, input.idx);
    
  
    
    
    
    
    
    
    return output;
}