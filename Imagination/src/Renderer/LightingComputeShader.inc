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
