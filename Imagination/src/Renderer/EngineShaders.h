#pragma once
#include <string>

namespace Shaders
{
    std::string TriangleVertexShader =
        R"(struct VIn
{
    float3 pos : POSITION;
    float3 col : COLOR;
    float2 uv  : TEXCOORD0;
};

struct UniformBuffer 
{
    matrix model;
    matrix view;
    matrix proj;
};
ConstantBuffer<UniformBuffer> ubo : register(b0, space0);

struct VOut
{
    float4 pos : SV_Position;
    float3 col : COLOR;
    float2 uv  : TEXCOORD0;
};

VOut main(VIn input)
{
    VOut output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.pos, 1.0))));
    output.col = input.col;
    output.uv = input.uv;
    
    return output;
})";

    std::string TriangleFragmentShader =
        R"(struct VOut
{
    float4 pos : SV_Position;
    float3 col : COLOR;
    float2 uv  : TEXCOORD0;
};

Texture2D _texture : register(t2, space0);
SamplerState _sampler : register(s2, space0);

float4 main(VOut input) : SV_Target
{
    return float4(input.col * _texture.Sample(_sampler, input.uv).rgb, 1.0);
})";

    std::string GBufferVertexShader = R"(struct VIn
{
    float3 pos : POSITION0;
    float3 nrm : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 tan : TANGENT;
    float3 col : COLOR;
};

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

[[vk::push_constant]]
struct GBufferPC
{
    matrix model;
    uint materialIndex;
} pc;

struct UniformBuffer
{
    matrix viewProj;
    matrix prevViewProj;
    matrix jitteredViewProj;
};
ConstantBuffer<UniformBuffer> ubo : register(b0, space0);

VOut main(VIn input)
{
    VOut output;
    output.pos = mul(ubo.jitteredViewProj, mul(pc.model, float4(input.pos, 1)));
    output.wPos = input.pos;
    output.uv = input.uv;
    
    float3x3 model3x3 = (float3x3) pc.model;
    
    output.nrm = normalize(mul(model3x3, input.nrm));
    output.tan.xyz = normalize(mul(model3x3, input.tan.xyz));
    output.tan.w = input.tan.w;
    output.col = input.col;

    output.cPos = mul(ubo.viewProj, mul(pc.model, float4(input.pos, 1)));
    output.pPos = mul(ubo.prevViewProj, mul(pc.model, float4(input.pos, 1)));
    
    return output;
})";

    std::string GBufferFragmentShader = R"(struct VOut
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
    float4 Emissi : SV_TARGET3;
    float2 Velocity : SV_TARGET4;
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
    int4 textureIndices0;
    int4 textureIndices1;
    float4 materialFactors;
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
        albedo *= materialTextures[baseColorTexture].Sample(materialSampler, input.uv);
    if (alphaMode == ALPHA_MASK)
        if (albedo.a < alphaCutoff) discard;
    if (alphaMode == ALPHA_BLEND)
        if (albedo.a < 0.5f) discard;
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
        emissive *= materialTextures[emissiveTexture].Sample(materialSampler, input.uv).rgb;
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
        worldNormal = -worldNormal;
    output.Albedo = float4(albedo.rgb, 1.f);
    output.Normal = float4(worldNormal * .5f + .5f, 1.f);
    output.MStuff = float4(metallic, roughness, ao, 1.0f);
    output.Emissi = float4(emissive, 1.f);
    output.Velocity = float2(ClipToUV(input.cPos) - ClipToUV(input.pPos));
    return output;
})";

    std::string LightingVertexShader = R"(struct VOut
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};

VOut main(uint VertexIndex : SV_VertexID)
{
    VOut output;
    output.UV = float2((VertexIndex << 1) & 2, VertexIndex & 2);
    output.Pos = float4(output.UV * 2.0f - 1.0f, 0.0f, 1.0f);
    return output;
})";

    std::string LightingFragmentShader = R"(struct VOut
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};

Texture2D _textures[] : register(t2, space0);
SamplerState _sampler : register(s2, space0);

float4 main(VOut input) : SV_TARGET
{
    return _textures[2].Sample(_sampler, input.UV);
})";

    std::string LightingComputeShader = R"(#define PI 3.14159265359
#define GBUFFER_ALBEDO   0
#define GBUFFER_NORMAL   1
#define GBUFFER_MATERIAL 2
#define GBUFFER_EMISSIVE 3
#define GBUFFER_DEPTH    4

[[vk::push_constant]]
struct GBufferPC
{
    matrix invViewProj;
    float3 camPos;
    uint width, height, pointLightCount;
} pc;

Texture2D<float4> gBuffer[5] : register(t2, space0);
RWTexture2D<float4> litScene : register(u3, space0);

float3 FresnelSchlick(float pCosTheta, float3 F0)
{
    return F0 + (1.f - F0) * pow(clamp(1.f - pCosTheta, 0.f, 1.f), 5.f);
}

float DistributionGGX(float3 N, float3 H, float pRoughness)
{
    float a = pRoughness * pRoughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.f);
    float NdotH2 = NdotH * NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.f) + 1.f);
    denom = PI * denom * denom;
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float pRoughness)
{
    float r = (pRoughness + 1.0);
    float k = (r * r) / 8.0;
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float pRoughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, pRoughness);
    float ggx1 = GeometrySchlickGGX(NdotL, pRoughness);
    return ggx1 * ggx2;
}

float3 PointLight(float3 pWorldPos, float3 N, float3 V, float3 pLightPos, float3 pLightColor, float pRange, float3 pAlbedo, float pMetallic, float pRoughness)
{
    float3 L = normalize(pLightPos - pWorldPos);
    float3 H = normalize(V + L);
    float distance = length(pLightPos - pWorldPos);
    float attenuation = 1.f / (distance * distance);
    if (pRange > .0f)
    {
        float rangeAttenuation = saturate(1.f - pow(distance / pRange, 4.f));
        rangeAttenuation *= rangeAttenuation;
        attenuation *= rangeAttenuation;
    }
    float3 radiance = pLightColor * attenuation;
    float3 F0 = float3(.04f, .04f, .04f);
    F0 = lerp(F0, pAlbedo, pMetallic);
    float3 F = FresnelSchlick(max(dot(H, V), 0.f), F0);
    float NDF = DistributionGGX(N, H, pRoughness);
    float G = GeometrySmith(N, V, L, pRoughness);
    float3 nominator = NDF * G * F;
    float denominator = 4.f * max(dot(N, V), 0.f) * max(dot(N, L), 0.f) + .001f;
    float3 specular = nominator / denominator;
    float3 kS = F;
    float3 kD = float3(1.f, 1.f, 1.f) - kS;
    kD *= 1.f - pMetallic;
    float NdotL = max(dot(N, L), 0.f);
    return (kD * pAlbedo / PI + specular) * radiance * NdotL;
}

float3 DirectionalLight(float3 N, float3 V, float3 pLightDir, float3 pLightColor, float3 pAlbedo, float pMetallic, float pRoughness)
{
    float3 L = normalize(-pLightDir);
    float3 H = normalize(V + L);
    float3 radiance = pLightColor;
    float3 F0 = float3(.04f, .04f, .04f);
    F0 = lerp(F0, pAlbedo, pMetallic);
    float3 F = FresnelSchlick(max(dot(H, V), 0.f), F0);
    float NDF = DistributionGGX(N, H, pRoughness);
    float G = GeometrySmith(N, V, L, pRoughness);
    float3 nominator = NDF * G * F;
    float denominator = 4.f * max(dot(N, V), 0.f) * max(dot(N, L), 0.f) + .001f;
    float3 specular = nominator / denominator;
    float3 kS = F;
    float3 kD = float3(1.f, 1.f, 1.f) - kS;
    kD *= 1.f - pMetallic;
    float NdotL = max(dot(N, L), 0.f);
    return (kD * pAlbedo / PI + specular) * radiance * NdotL;
}

float3 ReconstructWorldPosition(float2 pUV, float pDepth)
{
    float2 screenPos = pUV * 2.f - 1.f;
    float4 world = mul(pc.invViewProj, float4(screenPos, pDepth, 1.f));
    world /= world.w;
    return world.xyz;
}

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint2 pixel = DTid.xy;
    float2 uv = (float2(pixel) + .5f) / float2(pc.width, pc.height);
    float3 position = ReconstructWorldPosition(uv, gBuffer[GBUFFER_DEPTH].Load(int3(pixel, 0)).r);
    float metallic = gBuffer[GBUFFER_MATERIAL].Load(int3(pixel, 0)).r;
    float roughness = gBuffer[GBUFFER_MATERIAL].Load(int3(pixel, 0)).g;
    float occlusion = gBuffer[GBUFFER_MATERIAL].Load(int3(pixel, 0)).b;
    float3 N = normalize(gBuffer[GBUFFER_NORMAL].Load(int3(pixel, 0)).rgb * 2.f - 1.f);
    float3 V = normalize( pc.camPos.rgb - position);
    float3 Lo = float3(0.f, 0.f, 0.f);
    Lo += PointLight(position, N, V, float3(0.f, 0.f, 0.f), float3(100.f, 0.f, 0.f), 1000.f, gBuffer[GBUFFER_ALBEDO].Load(int3(pixel, 0)).rgb, metallic, roughness);
    Lo += PointLight(position, N, V, float3(1000.f, 0.f, 0.f), float3(0.f, 100.f, 100.f), 1000.f, gBuffer[GBUFFER_ALBEDO].Load(int3(pixel, 0)).rgb, metallic, roughness);
    Lo += PointLight(position, N, V, float3(-1000.f, 0.f, 0.f), float3(100.f, 0.f, 100.f), 1000.f, gBuffer[GBUFFER_ALBEDO].Load(int3(pixel, 0)).rgb, metallic, roughness);
    Lo += DirectionalLight(N, V, float3(-.2f, -1.f, -.3f), float3(1.f, 1.f, 1.f), gBuffer[GBUFFER_ALBEDO].Load(int3(pixel, 0)).rgb, metallic, roughness);
    float3 ambient = float3(.0f, .0f, .0f) * gBuffer[GBUFFER_ALBEDO].Load(int3(pixel, 0)).rgb * occlusion;
    float3 color = ambient + Lo;
    color = color / (color + float3(1.f, 1.f, 1.f));
    color = pow(color, float3(1.f / 2.2f, 1.f / 2.2f, 1.f / 2.2f));
    litScene[pixel] = float4(color, 1.f);
})";

    std::string TAAComputeShader = R"(#define LitScene 0
#define TAAHistory 1
#define Velocity 2
#define VelocityHistory 3

// Velocity rejection: UV-space scale for |prevVel - currVel| -> disocclusion [0,1].
// k=32 saturates at ~3% UV delta (Elo-style ghosting control).
static const float kVelocityDisocclusionScale = 32.0f;

[[vk::push_constant]]
struct TAAPC
{
    bool validHistory;
} pc;


Texture2D<float4> taaInput[4] : register(t2, space0);
RWTexture2D<float4> taaOutput : register(u3, space0);
SamplerState _sampler : register(s4, space0);

// 9-tap Catmull-Rom via bilinear-optimized fetches (MJP / UE4 style).
// Sharper history reconstruction than a single bilinear Sample.
float3 SampleHistoryCatmullRom(Texture2D<float4> tex, SamplerState samp, float2 uv, float2 texSize)
{
    float2 samplePos = uv * texSize;
    float2 texPos1 = floor(samplePos - 0.5f) + 0.5f;
    float2 f = samplePos - texPos1;

    float2 w0 = f * (-0.5f + f * (1.0f - 0.5f * f));
    float2 w1 = 1.0f + f * f * (-2.5f + 1.5f * f);
    float2 w2 = f * (0.5f + f * (2.0f - 1.5f * f));
    float2 w3 = f * f * (-0.5f + 0.5f * f);

    float2 w12 = w1 + w2;
    float2 offset12 = w2 / (w12 + 1e-5f);

    float2 texPos0 = texPos1 - 1.0f;
    float2 texPos3 = texPos1 + 2.0f;
    float2 texPos12 = texPos1 + offset12;

    texPos0 /= texSize;
    texPos3 /= texSize;
    texPos12 /= texSize;

    float3 result = 0.0f.xxx;
    result += tex.SampleLevel(samp, float2(texPos0.x,  texPos0.y),  0).rgb * (w0.x  * w0.y);
    result += tex.SampleLevel(samp, float2(texPos12.x, texPos0.y),  0).rgb * (w12.x * w0.y);
    result += tex.SampleLevel(samp, float2(texPos3.x,  texPos0.y),  0).rgb * (w3.x  * w0.y);

    result += tex.SampleLevel(samp, float2(texPos0.x,  texPos12.y), 0).rgb * (w0.x  * w12.y);
    result += tex.SampleLevel(samp, float2(texPos12.x, texPos12.y), 0).rgb * (w12.x * w12.y);
    result += tex.SampleLevel(samp, float2(texPos3.x,  texPos12.y), 0).rgb * (w3.x  * w12.y);

    result += tex.SampleLevel(samp, float2(texPos0.x,  texPos3.y),  0).rgb * (w0.x  * w3.y);
    result += tex.SampleLevel(samp, float2(texPos12.x, texPos3.y),  0).rgb * (w12.x * w3.y);
    result += tex.SampleLevel(samp, float2(texPos3.x,  texPos3.y),  0).rgb * (w3.x  * w3.y);

    return result;
}

// 3x3 max-magnitude velocity dilation (cheap silhouette / edge help).
float2 DilateVelocityMaxMagnitude(uint2 pixel, uint width, uint height)
{
    float2 bestVel = 0.0f.xx;
    float bestMag2 = -1.0f;
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            int2 p = clamp(int2(pixel) + int2(x, y), int2(0, 0), int2(width - 1, height - 1));
            float2 v = taaInput[Velocity].Load(int3(p, 0)).rg;
            float mag2 = dot(v, v);
            if (mag2 > bestMag2)
            {
                bestMag2 = mag2;
                bestVel = v;
            }
        }
    }
    return bestVel;
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 pixel = DTid.xy;
    uint width, height;
    taaOutput.GetDimensions(width, height);
    
    float2 uv = (float2(pixel) + .5f) / float2(width, height);

    // Dilate current velocity before reprojection (max-magnitude 3x3).
    float2 dilatedVel = DilateVelocityMaxMagnitude(pixel, width, height);
    float2 reprojectedUV = uv - dilatedVel;
    float3 currColor = taaInput[LitScene].Load(int3(pixel, 0)).rgb;
    
    float3 prevColor = currColor;
    //valid history - Catmull-Rom history sample (keep RGB AABB clamp below)
    if (pc.validHistory)
        prevColor = SampleHistoryCatmullRom(taaInput[TAAHistory], _sampler, reprojectedUV, float2(width, height));
    
    float3 minColor = 9999.f, maxColor = -9999.f;
    float3 neighborhoodSum = 0.0f.xxx;
    
    //color clamping (RGB AABB - YCoCg later) + 3x3 neighborhood blur accum
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            float3 color = taaInput[LitScene].Load(int3(clamp(pixel + int2(x, y), int2(0, 0), int2(width - 1, height - 1)), 0)).rgb;
            minColor = min(minColor, color);
            maxColor = max(maxColor, color);
            neighborhoodSum += color;
        }
    }
    float3 neighborhoodBlur = neighborhoodSum / 9.0f;
    
    float3 previousColorClamped = clamp(prevColor, minColor, maxColor);

    // Velocity rejection / disocclusion: compare previous-frame velocity at
    // reprojected UV against dilated current velocity.
    float disocclusion = 0.0f;
    if (pc.validHistory)
    {
        float2 prevVel = taaInput[VelocityHistory].SampleLevel(_sampler, reprojectedUV, 0).rg;
        disocclusion = saturate(kVelocityDisocclusionScale * length(prevVel - dilatedVel));
    }

    // Raise current weight with disocclusion; high-disocclusion path leans on
    // neighborhood-blurred current to reduce ghosting.
    float currWeight = lerp(0.1f, 1.0f, disocclusion);
    float3 currResolved = lerp(currColor, neighborhoodBlur, disocclusion);
    
    taaOutput[pixel] = float4(currResolved * currWeight + previousColorClamped * (1.0f - currWeight), 1.f);
}
)";

}
