struct VOut
{
    float4 pos : SV_Position;
    float3 wPos : POSITION;
    float3 nrm : NORMAL;
    float2 uv : TEXCOORD0;
    float4 tan : TANGENT;
    float3 col : COLOR;
    float4 cPos : POSITION1;
    float4 pPos : POSITION2;
};

struct FOut
{
    float4 Albedo : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 MStuff : SV_TARGET2;
    float4 Emissi : SV_TARGET3; //emissive
    float2 Velocity : SV_TARGET4;
    //float4 Albedo   : SV_TARGET2;
};

[[vk::push_constant]]
struct GBufferPC
{
    matrix model;
    uint materialIndex;
} pc;

#define ALPHA_OPAQUE 0
#define ALPHA_MASK   1
#define ALPHA_BLEND  2

struct Material
{
    float4 baseColorFactor;
    float4 emissiveFactor;

    // x = base color
		// y = metallic/roughness
		// z = emissive
		// w = normal
    int4 textureIndices0;

    // x = occlusion
		// y = alpha mode
		// z = double sided
		// w = unused
    int4 textureIndices1;

    		// x = metallic
		// y = roughness
		// z = alpha cutoff
		// w = normal scale
    float4 materialFactors;

    // x = occlusion strength
		// yzw = unused
    float4 extraFactors;
};

StructuredBuffer<Material> materialInfo : register(t1, space0);

Texture2D materialTextures[] : register(t0, space1);
SamplerState materialSampler : register(s0, space1);

float2 ClipToUV(float4 pClip)
{
    float2 ndc = pClip.xy / pClip.w;
    
    return ndc * .5f + .5f;
}

FOut main(VOut input, bool isFrontFace : SV_IsFrontFace)
{
    FOut output;

    Material m = materialInfo[pc.materialIndex];

    int baseColorTexture = m.textureIndices0.x;
    int metallicRoughnessTexture = m.textureIndices0.y;
    int emissiveTexture = m.textureIndices0.z;
    int normalTexture = m.textureIndices0.w;

    int occlusionTexture = m.textureIndices1.x;
    int alphaMode = m.textureIndices1.y;
    int doubleSided = m.textureIndices1.z;

    float metallicFactor = m.materialFactors.x;
    float roughnessFactor = m.materialFactors.y;
    float alphaCutoff = m.materialFactors.z;
    float normalTextureScale = m.materialFactors.w;

    float occlusionTextureStrength = m.extraFactors.x;
    
    float4 albedo = m.baseColorFactor;

    if (baseColorTexture > -1)
    {
        albedo *= materialTextures[baseColorTexture].Sample(materialSampler, input.uv);
    }

    if (alphaMode == ALPHA_MASK)
    {
        if (albedo.a < alphaCutoff)
            discard;
    }

    if (alphaMode == ALPHA_BLEND)
    {
        // temporary: discard low-alpha pixels in deferred pass
        if (albedo.a < 0.5f)
            discard;
    }

    float metallic = metallicFactor;
    float roughness = roughnessFactor;

    if (metallicRoughnessTexture > -1)
    {
        float4 mr = materialTextures[metallicRoughnessTexture].Sample(materialSampler, input.uv);

        roughness *= mr.g;
        metallic *= mr.b;
    }
    
    float ao = 1.0f;

    if (occlusionTexture > -1)
    {
        float sampledAO = materialTextures[occlusionTexture].Sample(materialSampler, input.uv).r;

        ao = lerp(1.0f, sampledAO, occlusionTextureStrength);
    }

    float3 emissive = m.emissiveFactor.rgb;

    if (emissiveTexture > -1)
    {
        float3 emissiveSample = materialTextures[emissiveTexture].Sample(materialSampler, input.uv).rgb;

        emissive *= emissiveSample;
    }
    
    float3 N = normalize(input.nrm);
    float3 worldNormal = N;
    if (normalTexture > -1)
    {
        float3 normal = materialTextures[normalTexture].Sample(materialSampler, input.uv).rgb * 2.f - 1.f;
        normal = normalize(normal * float3(normalTextureScale, normalTextureScale, 1.f));

        if (any(input.tan))
        {
            float3 T = normalize(input.tan.xyz);
            T = normalize(T - dot(T, N) * N);
            float3 B = cross(N, T) * input.tan.w;
            float3x3 TBN = float3x3(T, B, N);
            worldNormal = normalize(mul(normal, TBN));
        }
    }
    
    if (doubleSided != 0 && !isFrontFace)
    {
        worldNormal = -worldNormal;
    }

    output.Albedo = float4(albedo.rgb, 1.f);
    output.Normal = float4(worldNormal * .5f + .5f, 1.f);
    output.MStuff = float4(metallic, roughness, ao, 1.0f);
    output.Emissi = float4(emissive, 1.f);
    output.Velocity = float2(ClipToUV(input.cPos) - ClipToUV(input.pPos));

    return output;
}