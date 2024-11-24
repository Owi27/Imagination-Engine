struct InVert
{
    float3 pos : POSITION;
    float3 nrm : NORMAL;
    float2 uv0 : TEXCOORD_0;
};

struct OutVert
{
    float4 pos : SV_Position;
    float3 nrm : NORMAL;
    float2 uv0 : TEXCOORD_0;
    float4 currPos : TEXCOORD_1;
    float4 prevPos : TEXCOORD_2;
};

cbuffer Matrices : register(b0)
{
    matrix prevWorldViewProjection;
    matrix currWorldViewProjection;
    float deltaTime;
};

OutVert main(InVert iVert)
{
    OutVert oVert;
    oVert.currPos = mul(currWorldViewProjection, float4(iVert.pos, 1));
    oVert.prevPos = mul(prevWorldViewProjection, float4(iVert.pos, 1));
    
    oVert.pos = oVert.currPos;
    oVert.nrm = iVert.nrm;
    oVert.uv0 = iVert.uv0;
    
    return oVert;
}