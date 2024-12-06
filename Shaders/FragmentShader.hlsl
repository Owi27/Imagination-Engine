Texture2D textureposition : register(t1);
SamplerState samplerposition : register(s1);
Texture2D textureNormal : register(t2);
SamplerState samplerNormal : register(s2);
Texture2D textureAlbedo : register(t3);
SamplerState samplerAlbedo : register(s3);

cbuffer UniformBufferFinal : register(b0)
{
    float4 view;
};

float4 main(float2 inUV : TEXCOORD0) : SV_TARGET
{
   // float3 fragPos = textureposition.Sample(samplerposition, inUV).rgb;
    float3 fragPos = textureAlbedo.Sample(samplerAlbedo, inUV).rgb;
   // float3 V = view.xyz - fragPos;
  
    //float3 fragNrm = textureNormal.Sample(samplerNormal, inUV).rgb;
    
    return float4(fragPos, 1);
}