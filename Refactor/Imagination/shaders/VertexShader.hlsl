struct VertexIN
{
    float3 pos : POSITION;
    float4 col : COLOR;
};

struct VertexOUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

VertexOUT main(VertexIN input)
{
    VertexOUT output;
    
    output.pos = float4(input.pos, 1.f);
    output.col = input.col;
    
    return output;
}