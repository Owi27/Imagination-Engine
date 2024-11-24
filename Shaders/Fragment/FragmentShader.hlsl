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

Texture2D _texure : register(t0, space1);
SamplerState _sampler : register(s0, space1);

float4 main(OutVert oVert) : SV_TARGET
{
    int numOfSamples = 8;

    //return _texure.Sample(_sampler, oVert.uv);
    
    float3 curr = oVert.currPos.xyz / oVert.currPos.w;
    float3 prev = oVert.prevPos.xyz / oVert.prevPos.w;
    
    float3 velocity = (curr - prev) / deltaTime;
    return float4(velocity, 1);
    
    float4 color = _texure.Sample(_sampler, oVert.uv0);
    
    oVert.uv0 += velocity.xy;
    
    for (int i = 1; i < numOfSamples; i++, oVert.uv0 += velocity.xy)
    {
        float4 currColor = color;
        color += currColor;
        
    }
    
        return color/numOfSamples;
}