#include "pch.h"

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
ConstantBuffer<UniformBuffer> ubo;

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

Texture2D _texure : register(t1, space0);
SamplerState _sampler : register(s1, space0);

float4 main(VOut input) : SV_Target
{
    return float4(input.col * _texure.Sample(_sampler, input.uv).rgb, 1.0);
    //return _texure.Sample(_sampler, input.uv);
})";
}