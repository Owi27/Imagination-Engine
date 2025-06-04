//Texture2D textureposition : register(t1);
//SamplerState samplerposition : register(s1);
//Texture2D textureNormal : register(t2);
//SamplerState samplerNormal : register(s2);
//Texture2D textureAlbedo : register(t3);
//SamplerState samplerAlbedo : register(s3);

Texture2D materialTexures[] : register(t0, space1);
SamplerState materialSampler : register(s0, space1);

[[vk::input_attachment_index(0)]][[vk::binding(1)]] SubpassInput posAttachment;
[[vk::input_attachment_index(1)]][[vk::binding(2)]] SubpassInput nrmAttachment;
[[vk::input_attachment_index(2)]][[vk::binding(3)]] SubpassInput albAttachment;
[[vk::input_attachment_index(3)]][[vk::binding(4)]] SubpassInput mpAttachment;
[[vk::input_attachment_index(4)]][[vk::binding(5)]] SubpassInput depthAttachment;
[[vk::input_attachment_index(5)]][[vk::binding(6)]] SubpassInput skybox;

float3 FresnelSchlick(float cosTheta, float3 F0);
float DistributionGGX(float3 N, float3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(float3 N, float3 V, float3 L, float roughness);

#define PI 3.14159265359
#define lightCount 10

struct Light
{
    float3 pos;
    float3 col;
    float radius;
};

cbuffer UniformBufferFinal : register(b0)
{
    Light lights[10];
    float4 camPos;
    matrix viewProj;
};

float4 main(float2 inUV : TEXCOORD0) : SV_TARGET
{
    float3 fragPos = posAttachment.SubpassLoad().rgb; //    normalize(textureposition.Sample(samplerposition, inUV).rgb);
    float3 normal = nrmAttachment.SubpassLoad().rgb; //normalize(textureNormal.Sample(samplerNormal, inUV).rgb);
    float3 albedo = pow(albAttachment.SubpassLoad().rgb, 2.2); //textureAlbedo.Sample(samplerAlbedo, inUV).rgb;
    float specular = albAttachment.SubpassLoad().a; //textureAlbedo.Sample(samplerAlbedo, inUV).a;
    
    float roughness = nrmAttachment.SubpassLoad().a;
    float metallic = mpAttachment.SubpassLoad().r;
    float emissive = mpAttachment.SubpassLoad().g;
    float ao = mpAttachment.SubpassLoad().b;
    float mIdx = mpAttachment.SubpassLoad().a;
        
    if (fragPos.r < 0.999)
    {
        return skybox.SubpassLoad();
    }
    
    float3 nrm = normal;
    nrm = nrm * 2.0 - 1.0; // remap
    
   // return float4(nrm, 1);
    //return albAttachment.SubpassLoad();
    float3 N = normalize(normal);
    float3 V = normalize(camPos.xyz - fragPos);
        
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
        
        //direct lighting
    float3 Lo = float3(0, 0, 0);
    for (int i = 0; i < lightCount; i++)
    {
        float3 L = normalize(lights[i].pos - fragPos);
        float3 H = normalize(V + L);
            
        float distance = length(lights[i].pos - fragPos);
        float attenuation = 1.f / (distance * distance);
        float3 radiance = lights[i].col * attenuation;

            
        float3 F = FresnelSchlick(max(dot(H, V), 0.f), F0);
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
            
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        float3 specular = numerator / denominator;
            
        float3 kS = F;
        float3 kD = float3(1.f, 1.f, 1.f) - kS;
  
        kD *= 1.0 - metallic;
            
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
        
    float3 ambient = float3(0.3, 0.3, 0.3) * albedo * ao;
    float3 color = ambient + Lo;
    color = color / (color + float3(1.f, 1.f, 1.f));
    color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
        
        
    return float4(color, 1);
    
    
    
    
    
    //pbr
    float4 output;
    
    
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 = (1.f - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}