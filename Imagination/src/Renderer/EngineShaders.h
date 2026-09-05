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
    //return _texure.Sample(_sampler, input.uv);
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
    //float3x3 TBN : TEXCOORD1;
    //int idx : INDEX;
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
    output.nrm = input.nrm;
    output.tan = input.tan;
    output.col = input.col;

    output.cPos = mul(ubo.viewProj, mul(pc.model, float4(input.pos, 1)));
    output.pPos = mul(ubo.prevViewProj, mul(pc.model, float4(input.pos, 1)));
    
    //output.idx = id;

//VOut output;
//    output.pos = float4(input.pos.xy, 0.5, 1.0); // Bypass matrices
//    output.uv = input.uv;
//    output.nrm = input.nrm;
//    return output;
    
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

    float metallic  = metallicFactor;
    float roughness = roughnessFactor;

    if (metallicRoughnessTexture > -1)
    {
        float4 mr = materialTextures[metallicRoughnessTexture].Sample(materialSampler, input.uv);

        roughness *= mr.g;
        metallic  *= mr.b;
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

    float3 normal = float3(0.f, 0.f, 1.f);
    if (normalTexture > -1)
    {
        normal = materialTextures[normalTexture].Sample(materialSampler, input.uv).rgb * 2.f - 1.f;
        normal = normalize(normal * float3(normalTextureScale, normalTextureScale, 1.f));
    }
    float3 T = normalize(input.tan.xyz);
    float3 N = normalize(input.nrm);
    T = normalize(T - dot(T, N) * N); // Gram-Schmidt
    float3 B = cross(N, T) * input.tan.w; // handedness
    float3x3 TBN = float3x3(T, B, N);
    float3 worldNormal = normalize(mul(normal, TBN));
    //float3 worldNormal = normalize(T * normal.x + B * normal.y + N * normal.z);

    if (doubleSided != 0 && !isFrontFace)
    {
        worldNormal = -worldNormal;
    }

    //velocity
    //float2 currNDC = input.cPos.xy / input.cPos.w, prevNDC = input.pPos.xy / input.pPos.w;
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

//[[vk::input_attachment_index(0)]][[vk::binding(1)]] SubpassInput posAttachment;

Texture2D _textures[] : register(t2, space0);
SamplerState _sampler : register(s2, space0);

float4 main(VOut input) : SV_TARGET
{
    return _textures[2].Sample(_sampler, input.UV);
    //return float4(1.f, 0.f, 0.f, 1.f);
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
    float3 L = normalize(pLightPos - pWorldPos); //direction from surface to light
    float3 H = normalize(V + L); //halfway vec between view and light dir.
    
    float distance = length(pLightPos - pWorldPos); //distance between surface and point light
    float attenuation = 1.f / (distance * distance); //light becomes a quarter as strong as distance increases
    
    if (pRange > .0f) //if no range use regular attenuation
    {
        //fade light as it approaches range
        float rangeAttenuation = saturate(1.f - pow(distance / pRange, 4.f));
        rangeAttenuation *= rangeAttenuation; //smoother fade to 0
        attenuation *= rangeAttenuation; //apply range to attenuation
    }
    
    float3 radiance = pLightColor * attenuation; //reduces light color based on dist/range
    
    float3 F0 = float3(.04f, .04f, .04f); //mats specular reflectance
    F0 = lerp(F0, pAlbedo, pMetallic); //lerp between non-metals reflectivity and metallic reflectivity
    
    float3 F = FresnelSchlick(max(dot(H, V), 0.f), F0); //controls light reflection
    float NDF = DistributionGGX(N, H, pRoughness); //surface facets along H
    float G = GeometrySmith(N, V, L, pRoughness); //facets masking another from light/viewer
    
    float3 nominator = NDF * G * F; //numerator of the Cook-Torrance specular BRDF
    float denominator = 4.f * max(dot(N, V), 0.f) * max(dot(N, L), 0.f) + .001f; //denominator of the Cook-Torrance specular BRDF
    float3 specular = nominator / denominator; //mats specular 
    
    float3 kS = F; //percent of incoming light reflected as specular
    float3 kD = float3(1.f, 1.f, 1.f) - kS; //light remaining for diffuse reflection
    kD *= 1.f - pMetallic; //metals dont have a traditional diffuse
    
    float NdotL = max(dot(N, L), 0.f);
    
    return (kD * pAlbedo / PI + specular) * radiance * NdotL;
}

float3 DirectionalLight(float3 N, float3 V, float3 pLightDir, float3 pLightColor, float3 pAlbedo, float pMetallic, float pRoughness)
{
    float3 L = normalize(-pLightDir); //direction from light to surface
    float3 H = normalize(V + L); //halfway vec between view and light dir.
        
    float3 radiance = pLightColor; //directional light so no fade away
    
    float3 F0 = float3(.04f, .04f, .04f); //mats specular reflectance
    F0 = lerp(F0, pAlbedo, pMetallic); //lerp between non-metals reflectivity and metallic reflectivity
    
    float3 F = FresnelSchlick(max(dot(H, V), 0.f), F0); //controls light reflection
    float NDF = DistributionGGX(N, H, pRoughness); //surface facets along H
    float G = GeometrySmith(N, V, L, pRoughness); //facets masking another from light/viewer
    
    float3 nominator = NDF * G * F; //numerator of the Cook-Torrance specular BRDF
    float denominator = 4.f * max(dot(N, V), 0.f) * max(dot(N, L), 0.f) + .001f; //denominator of the Cook-Torrance specular BRDF
    float3 specular = nominator / denominator; //mats specular 
    
    float3 kS = F; //percent of incoming light reflected as specular
    float3 kD = float3(1.f, 1.f, 1.f) - kS; //light remaining for diffuse reflection
    kD *= 1.f - pMetallic; //metals dont have a traditional diffuse
    
    float NdotL = max(dot(N, L), 0.f);
    
    return (kD * pAlbedo / PI + specular) * radiance * NdotL;
}

/* Shadows */
//float PointLightShadow(float pShadowFarPlane, float3 pWorldPos, float3 pLightPos, TextureCube<float4> pShadowMap)
//{
//    float3 fragToLight = pWorldPos - pLightPos;
//    float closestDepth = pShadowMap.SampleLevel( /*$(Sampler:point:point:point:clamp)*/, normalize(fragToLight), 0).r * pShadowFarPlane;
//    float currentDepth = length(fragToLight);
//    float bias = .05f;
    
//    float shadow = currentDepth - bias > closestDepth ? 1.f : 0.f;
    
//    return shadow;
//}

float3 ReconstructWorldPosition(float2 pUV, float pDepth)
{
	//screen uv to ndc
    float2 screenPos = pUV * 2.f - 1.f;
    
	//ndc & depth -> viewSpace pos
	//view to world
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
    
    float3 N = normalize(gBuffer[GBUFFER_NORMAL].Load(int3(pixel, 0)).rgb * 2.f - 1.f); //-1-1
    float3 V = normalize( pc.camPos.rgb - position);

    //direct lighting
    float3 Lo = float3(0.f, 0.f, 0.f);
    
    Lo += PointLight(position, N, V, float3(0.f, 0.f, 0.f), float3(100.f, 0.f, 0.f), 1000.f, gBuffer[GBUFFER_ALBEDO].Load(int3(pixel, 0)).rgb, metallic, roughness);
    Lo += PointLight(position, N, V, float3(1000.f, 0.f, 0.f), float3(0.f, 100.f, 100.f), 1000.f, gBuffer[GBUFFER_ALBEDO].Load(int3(pixel, 0)).rgb, metallic, roughness);
    Lo += PointLight(position, N, V, float3(-1000.f, 0.f, 0.f), float3(100.f, 0.f, 100.f), 1000.f, gBuffer[GBUFFER_ALBEDO].Load(int3(pixel, 0)).rgb, metallic, roughness);
    //for (int i = 0; i < pc.pointLightCount; i++)
    //{
        
    //}

    //Directional Light
    //Lo += DirectionalLight(N, V, float3(-.2f, -1.f, -.3f), float3(1.f, 1.f, 1.f), gBuffer[GBUFFER_ALBEDO].Load(int3(pixel, 0)).rgb, metallic, roughness);
    
    float3 ambient = float3(.0f, .0f, .0f) * gBuffer[GBUFFER_ALBEDO].Load(int3(pixel, 0)).rgb * occlusion;
    float3 color = ambient + Lo;
    
    color = color / (color + float3(1.f, 1.f, 1.f));
    color = pow(color, float3(1.f / 2.2f, 1.f / 2.2f, 1.f / 2.2f));

    litScene[pixel] = float4(color, 1.f);
})";

}