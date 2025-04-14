//Texture2D finalTexture : register(t0);
//SamplerState finalTextureSampler : register(s0);

[[vk::input_attachment_index(0)]][[vk::binding(0)]] SubpassInput finalTexture;

float4 main(float2 inUV : TEXCOORD0) : SV_TARGET
{
    return finalTexture.SubpassLoad();
}