struct VSInput
{
    float2 Pos : POSITION0;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR0;
};

struct PushConstants
{
    float2 scale;
    float2 translate;
};

[[vk::push_constant]]
PushConstants pushConstants;

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR0;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    output.UV = input.UV;
    output.Color = input.Color;
    output.Pos = float4(input.Pos * pushConstants.scale + pushConstants.translate, 0.0, 1.0);
    return output;
}