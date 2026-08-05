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
    float3 wPos : POSITION;
    float3 nrm : NORMAL;
    float2 uv : TEXCOORD0;
    float4 tan : TANGENT;
    float3 col : COLOR;
    //float3x3 TBN : TEXCOORD1;
    //int idx : INDEX;
};

[[vk::push_constant]]
struct GBufferPC
{
    matrix model;
    uint materialIndex;
} pc;

struct UniformBuffer
{
    matrix view;
    matrix proj;
};
ConstantBuffer<UniformBuffer> ubo : register(b0, space0);

VOut main(VIn input)
{
    VOut output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(pc.model, float4(input.pos, 1))));
    output.wPos = input.pos;
    output.uv = input.uv;
    output.nrm = input.nrm;
    output.tan = input.tan;
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
    float3 wPos : POSITION;
    float3 nrm : NORMAL;
    float2 uv : TEXCOORD0;
    float4 tan : TANGENT;
    float3 col : COLOR;
};

struct FOut
{
    float4 Albedo : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 Mstuff : SV_TARGET2;
    //float4 Albedo   : SV_TARGET2;
};

[[vk::push_constant]]
struct GBufferPC
{
    matrix model;
    uint materialIndex;
} pc;

#define ALPHA_OPAQUE 0
#define ALPHA_MASK   1
#define ALPHA_BLEND  2

struct Material
{
    float4 baseColorFactor;
    float4 emissiveFactor;

    int4 textureIndices0;
    int4 textureIndices1;

    float4 materialFactors;
    float4 extraFactors;
};

StructuredBuffer<Material> materialInfo : register(t1, space0);

Texture2D materialTextures[] : register(t0, space1);
SamplerState materialSampler : register(s0, space1);

FOut main(VOut input)
{
    FOut output;

    Material m = materialInfo[pc.materialIndex];
    int baseColorTexture = m.textureIndices0.x;
    int metallicRoughnessTexture = m.textureIndices0.y;
    int emissiveTexture = m.textureIndices0.z;
    int normalTexture = m.textureIndices0.w;
    int occlusionTexture = m.textureIndices1.x;
    int alphaMode = m.textureIndices1.y;
    int doubleSided = m.textureIndices1.z;
    float metallicFactor = m.materialFactors.x;
    float roughnessFactor = m.materialFactors.y;
    float alphaCutoff = m.materialFactors.z;
    float normalTextureScale = m.materialFactors.w;
    float occlusionTextureStrength = m.extraFactors.x;
    
    float3 T = normalize(input.tan.xyz);
    float3 N = normalize(input.nrm);
    T = normalize(T - dot(T, N) * N); // Gram-Schmidt
    float3 B = cross(N, T) * input.tan.w; // handedness
    float3x3 TBN = float3x3(T, B, N);
    
    float3 worldNormal = normalize(mul(normalize(input.nrm), TBN));

    float4 albedo = m.baseColorFactor;

    if (baseColorTexture > -1)
    {
        albedo *= materialTextures[baseColorTexture].Sample(materialSampler, input.uv);
    }

    if (alphaMode == ALPHA_MASK)
    {
        if (albedo.a < alphaCutoff)
            discard;
    }

    if (alphaMode == ALPHA_BLEND)
    {
        // temporary: discard low-alpha pixels in deferred pass
        if (albedo.a < 0.5f)
            discard;
    }

    output.Albedo = float4(albedo.rgb, 1.f);
    //output.Albedo = float4(worldNormal * .5f + .5f, 1.f);
    //output.Albedo = float4(albedo.rgb, 1.0f);

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

    std::string LightingComputeShader = R"([numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
})";

}