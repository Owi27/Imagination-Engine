static const float2 positions[3] =
{
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
};
static const float3 colors[3] =
{
    float3(204.f/255.f, 180.f/255.f, 184.f/255.f),
    float3(1.f, 184.f/255f, 198f/255f),
    float3(51f/255f, 49f/255f, 60f/255f)
};

struct VIn
{
    float2 pos : POSITION;
    float3 col : COLOR;
    float2 uv : TEXCOORD0
};

struct VOut
{
    float4 pos : SV_Position;
    float3 col : COLOR;
};

VOut main(VIn input)
{
    VOut output;
    output.pos = float4(input.pos, 0.0, 1.0);
    output.col = input.col;
    
    return output;
}

struct VOut
{
    float4 pos : SV_Position;
    float3 col : COLOR;
};

float4 main(VOut input) : SV_Target
{
    return float4(input.col, 1.0);
}