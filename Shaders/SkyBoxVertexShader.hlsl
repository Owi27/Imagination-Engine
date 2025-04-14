cbuffer UniformBuffer : register(b0)
{
    matrix projection, model;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float3 uvw : TEXCOORD0;
};

VSOutput main(float3 pos : POSITION)
{
    VSOutput output;
    output.uvw = pos;
    
    //convert cubemap coords into vulkan coord space
    output.uvw.xy *= -1.f;
    
    //remove translation view matrix
    matrix view = model;
    view[0][3] = 0.f;
    view[1][3] = 0.f;
    view[2][3] = 0.f;
    output.pos = mul(projection, mul(view, float4(pos, 1)));
    
    return output;
}