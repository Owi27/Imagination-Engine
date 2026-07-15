#pragma once
#include <string>

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

Texture2D _texture : register(t2, space0);
SamplerState _sampler : register(s2, space0);

float4 main(VOut input) : SV_Target
{
    return float4(input.col * _texture.Sample(_sampler, input.uv).rgb, 1.0);
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
    int idx : INDEX;
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
    
    output.idx = id;

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
};

struct FOut
{
    float4 Position : SV_TARGET0;
    float4 Normal   : SV_TARGET1;
    float4 Albedo   : SV_TARGET2;
};

[[vk::push_constant]]
struct GBufferPC
{
    uint materialIndex;
} pc;

#define ALPHA_OPAQUE 0
#define ALPHA_MASK   1
#define ALPHA_BLEND  2

struct Material
{
    float4 baseColorFactor;
    int baseColorTexture;
    float metallicFactor;
    float roughnessFactor;
    int metallicRoughnessTexture;
    int emissiveTexture;
    float3 emissiveFactor;
    int alphaMode;
    float alphaCutoff;
    int doubleSided;
    int normalTexture;
    float normalTextureScale;
    int occlusionTexture;
    float occlusionTextureStrength;
    int _pad;
};

StructuredBuffer<Material> materialInfo : register(t1, space0);

Texture2D materialTextures[] : register(t2, space0);
SamplerState materialSampler : register(s2, space0);

FOut main(VOut input)
{
    FOut output;

    Material m = materialInfo[pc.materialIndex];

    float4 albedo = m.baseColorFactor;

    if (m.baseColorTexture > -1)
    {
        albedo *= materialTextures[m.baseColorTexture].Sample(materialSampler, input.uv);
    }

    if (m.alphaMode == ALPHA_MASK)
    {
        if (albedo.a < m.alphaCutoff)
            discard;
    }

    if (m.alphaMode == ALPHA_BLEND)
    {
        // temporary: discard low-alpha pixels in deferred pass
        if (albedo.a < 0.5f)
            discard;
    }

    output.Position = input.pos;
    output.Normal = float4(normalize(input.nrm), 1.0f);
    output.Albedo = float4(albedo.rgb, 1.0f);

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

    std::string LightingFragmentShader = R"(struct VOut
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};

//[[vk::input_attachment_index(0)]][[vk::binding(1)]] SubpassInput posAttachment;

Texture2D _textures[] : register(t2, space0);
SamplerState _sampler : register(s2, space0);

float4 main(VOut input) : SV_TARGET
{
    return _textures[2].Sample(_sampler, input.UV);
    //return float4(1.f, 0.f, 0.f, 1.f);
})";

    std::string LightingComputeShader = R"(struct VOut
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};

//[[vk::input_attachment_index(0)]][[vk::binding(1)]] SubpassInput posAttachment;

Texture2D _textures[] : register(t2, space0);
SamplerState _sampler : register(s2, space0);

float4 main(VOut input) : SV_TARGET
{
    return _textures[2].Sample(_sampler, input.UV);
    //return float4(1.f, 0.f, 0.f, 1.f);
})";

}