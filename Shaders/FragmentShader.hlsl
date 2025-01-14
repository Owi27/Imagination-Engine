Texture2D textureposition : register(t1);
SamplerState samplerposition : register(s1);
Texture2D textureNormal : register(t2);
SamplerState samplerNormal : register(s2);
Texture2D textureAlbedo : register(t3);
SamplerState samplerAlbedo : register(s3);

struct Light
{
    float3 pos;
    float3 col;
    float radius;
};

cbuffer UniformBufferFinal : register(b0)
{
    Light lights[10];
    float4 view;
    matrix viewProj;
};

float4 main(float2 inUV : TEXCOORD0) : SV_TARGET
{
    float3 fragPos = normalize(textureposition.Sample(samplerposition, inUV).rgb);
    float3 normal = normalize(textureNormal.Sample(samplerNormal, inUV).rgb);
    float3 albedo = textureAlbedo.Sample(samplerAlbedo, inUV).rgb;
    float specular = textureAlbedo.Sample(samplerAlbedo, inUV).a;
  
    //return float4(normalize(fragPos), 1.0); // Visualize position
  // return float4(normal, 1.0); // Visualize normal
   //return float4(albedo.rgb, 1.0); // Visualize albedo
    
   // return float4(lights[0].col, 1);
#define lightCount 4
#define ambient 0
    //float3 fragcolor;
    
    
    
    float3 fragcolor = albedo * 0;
    float3 viewDir = normalize(view.xyz - fragPos);
    for (int i = 0; i < 10; i++)
    {
        //float4 lightPos = mul(viewProj, float4(lights[i].pos, 1));
        //diffuse
        float3 lightDir = normalize(lights[i].pos - fragPos);
        float3 diffuse = max(dot(normal, lightDir), 0.0) * albedo * lights[i].col;
        
        //attenuation
        float dist = length(lights[i].pos - fragPos);
        float attenuation = saturate(1.0 - (dist / lights[i].radius));
        diffuse *= attenuation;
        
        //accumulate the light contribution
        fragcolor += diffuse;
    }
   
  
    return float4(fragcolor, 1);
	// Ambient part
    fragcolor = albedo.rgb * ambient;

    for (int i = 0; i < lightCount; ++i)
    {
		// Vector to light
        float3 L = lights[i].pos.xyz - fragPos;

		// Distance from light to fragment position
        float dist = length(L);

		// Viewer to fragment
        float3 V = view.xyz - fragPos;
        V = normalize(V);

		//if(dist < lights[i].radius)
		{
			// Light to fragment
            L = normalize(L);

			// Attenuation
            float atten = lights[i].radius / (pow(dist, 2.0) + 1.0);

			// Diffuse part
            float3 N = normalize(normal);
            float NdotL = max(0.0, dot(N, L));
            float3 diff = lights[i].col * albedo.rgb * NdotL * atten;

			// Specular part
			// Specular map values are stored in alpha of albedo mrt
            float3 R = reflect(-L, N);
            float NdotR = max(0.0, dot(R, V));
            float3 spec = lights[i].col * specular * pow(NdotR, 16.0) * atten;

            fragcolor += diff + spec;
        }
    }

    return float4(fragcolor, 1.0);
   // return float4(fragPos, 1);
}