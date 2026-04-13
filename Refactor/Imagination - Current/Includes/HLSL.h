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
ConstantBuffer<UniformBuffer> ubo : register(b0, space0);

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

Texture2D _texure : register(t2, space0);
SamplerState _sampler : register(s2, space0);

float4 main(VOut input) : SV_Target
{
    return float4(input.col * _texure.Sample(_sampler, input.uv).rgb, 1.0);
    //return _texure.Sample(_sampler, input.uv);
})";

    std::string GBufferVertexShader = R"(struct VIn
{
    float3 pos : POSITION0;
    float3 nrm : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 tan : TANGENT;
    float3 col : COLOR;
};

struct VOut
{
    float4 pos : SV_Position;
    float3 nrm : NORMAL;
    float2 uv : TEXCOORD0;
    float3 tan : TANGENT;
    float3 col : COLOR;
    //float3x3 TBN : TEXCOORD1;
    //int idx : INDEX;
};

struct UniformBuffer
{
    matrix model;
    matrix view;
    matrix proj;
};
ConstantBuffer<UniformBuffer> ubo : register(b0, space0);

VOut main(VIn input, uint id : SV_InstanceID)
{
    VOut output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.pos, 1))));
    output.uv = input.uv;
    output.nrm = normalize(input.nrm);
    output.tan = input.tan.rgb;
    output.col = input.col;
    
    //output.idx = id;

//VOut output;
//    output.pos = float4(input.pos.xy, 0.5, 1.0); // Bypass matrices
//    output.uv = input.uv;
//    output.nrm = input.nrm;
//    return output;
    
    return output;
})";

    std::string GBufferFragmentShader = R"(struct VOut
{
    float4 pos : SV_Position;
    float3 nrm : NORMAL;
    float2 uv : TEXCOORD0;
    float3 tan : TANGENT;
    float3 col : COLOR;
    //float3x3 TBN : TEXCOORD1;
    //int idx : INDEX;
};

struct FOut
{
    float4 Position : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 Albedo : SV_TARGET2;
    //float4 MProperties : SV_TARGET3;
};

FOut main(VOut input)
{
    FOut output;
    output.Position = float4(input.pos.rgb, 1.f);
    output.Normal = float4(input.nrm, 1.f);
    output.Albedo = float4(0.f, 0.f, 1.f, 1.f);

    return output;
})";

    std::string LightingVertexShader = R"(struct VOut
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};

VOut main(uint VertexIndex : SV_VertexID)
{
    VOut output;
    output.UV = float2((VertexIndex << 1) & 2, VertexIndex & 2);
    output.Pos = float4(output.UV * 2.0f - 1.0f, 0.0f, 1.0f);
    
    return output;
})";

    std::string LightingFragmentShader = R"(float4 main() : SV_TARGET
{
    return float4(1.f, 0.f, 0.f, 1.f);
})";

}