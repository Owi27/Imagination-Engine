#define PI 3.14159265359
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
    screenPos.y = -screenPos.y;
    
	//ndc & depth -> viewSpace pos
	//view to world
    float4 world = mul(float4(screenPos, pDepth, 1.f), pc.invViewProj);
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
    
    Lo += PointLight(position, N, V, float3(0.f, 0.f, 0.f), float3(1.f, 0.f, 0.f), 10.f, gBuffer[GBUFFER_ALBEDO].Load(int3(pixel, 0)).rgb, metallic, roughness);
    //for (int i = 0; i < pc.pointLightCount; i++)
    //{
        
    //}

    //Directional Light
    Lo += DirectionalLight(N, V, float3(-.2f, -1.f, -.3f), float3(1.f, 1.f, 1.f), gBuffer[GBUFFER_ALBEDO].Load(int3(pixel, 0)).rgb, metallic, roughness);
    
    float3 ambient = float3(.0f, .0f, .0f) * gBuffer[GBUFFER_ALBEDO].Load(int3(pixel, 0)).rgb * occlusion;
    float3 color = ambient + Lo;
    
    color = color / (color + float3(1.f, 1.f, 1.f));
    color = pow(color, float3(1.f / 2.2f, 1.f / 2.2f, 1.f / 2.2f));

    litScene[pixel] = float4(color, 1.f);
}