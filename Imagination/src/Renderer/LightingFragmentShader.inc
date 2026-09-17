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
