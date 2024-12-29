Texture2D textureposition : register(t1);
SamplerState samplerposition : register(s1);
Texture2D textureNormal : register(t2);
SamplerState samplerNormal : register(s2);
Texture2D textureAlbedo : register(t3);
SamplerState samplerAlbedo : register(s3);

struct Light
{
    float4 pos;
    float3 col;
    float radius;
};

cbuffer UniformBufferFinal : register(b0)
{
    Light lights[4];
    float4 view;
};

float4 main(float2 inUV : TEXCOORD0) : SV_TARGET
{
    float3 fragPos = textureposition.Sample(samplerposition, inUV).rgb;
    float3 normal = normalize(textureNormal.Sample(samplerNormal, inUV).rgb);
    float4 albedo = textureAlbedo.Sample(samplerAlbedo, inUV);
  
   //return float4(fragPos, 1.0); // Visualize position
  // return float4(normal, 1.0); // Visualize normal
   //return float4(albedo.rgb, 1.0); // Visualize albedo
    
    float3 fragcolor;

#define lightCount 1
#define ambient 0

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
            float3 spec = lights[i].col * albedo.a * pow(NdotR, 16.0) * atten;

            fragcolor += diff + spec;
        }
    }

    return float4(fragcolor, 1.0);
   // return float4(fragPos, 1);
}