Texture2D fontTexture : register(t0);
SamplerState fontSampler : register(s0);

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR0;
};

float4 main(VSOutput input) : SV_TARGET
{
    return input.Color * fontTexture.Sample(fontSampler, input.UV);
}