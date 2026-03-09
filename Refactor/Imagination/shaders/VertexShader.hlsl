struct VertexIN
{
    float3 pos : POSITION;
    float3 nrm : NORMAL;
    float2 uv : TEXCOORD;
    float4 tan : TANGENT;
};

StructuredBuffer<VertexIN> vertices : register(t1);

struct VertexOUT
{
    float4 pos : SV_POSITION;
    float3 nrm : NORMAL;
    float2 uv : TEXCOORD0;
    float4 tan : TANGENT;
};

VertexOUT main(uint vertexID : SV_VertexID)
{
    VertexOUT output;
    
    float4x4 world = float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    
    output.pos = mul(world, float4(vertices[vertexID].pos, 1.f));
    output.nrm = vertices[vertexID].nrm;
    output.uv = vertices[vertexID].uv;
    output.tan = vertices[vertexID].tan;
    
    return output;
}