TextureCube textureCubeMap : register(t1);
SamplerState samplerCubeMap : register(s1);

struct VSOutput
{
    float4 pos : SV_POSITION;
    float3 uvw : TEXCOORD0;
};

float4 main(VSOutput output) : SV_TARGET
{
    return textureCubeMap.Sample(samplerCubeMap, output.uvw);
}