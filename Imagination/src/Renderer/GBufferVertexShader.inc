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
    float4 cPos : POSITION1;
    float4 pPos : POSITION2;
};

[[vk::push_constant]]
struct GBufferPC
{
    matrix model;
    uint materialIndex;
    uint pad0, pad1, pad2;
    float3x4 prevModel;
} pc;

struct UniformBuffer
{
    matrix viewProj;
    matrix prevViewProj;
    matrix jitteredViewProj;
};
ConstantBuffer<UniformBuffer> ubo : register(b0, space0);

VOut main(VIn input)
{
    VOut output;
    output.pos = mul(ubo.jitteredViewProj, mul(pc.model, float4(input.pos, 1)));
    output.wPos = input.pos;
    output.uv = input.uv;
    
    float3x3 model3x3 = (float3x3) pc.model;
    
    output.nrm = normalize(mul(model3x3, input.nrm));
    output.tan.xyz = normalize(mul(model3x3, input.tan.xyz));
    output.tan.w = input.tan.w;
    output.col = input.col;

    output.cPos = mul(ubo.viewProj, mul(pc.model, float4(input.pos, 1)));
    float3 prevWorld = mul(pc.prevModel, float4(input.pos, 1));
    output.pPos = mul(ubo.prevViewProj, float4(prevWorld, 1));
    
    return output;
})";
