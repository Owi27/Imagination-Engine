struct VSInput
{
    float3 pos : POSITION0;
    float3 nrm : NORMAL0;
    float2 uv0 : TEXCOORD0;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 nrm : NORMAL0;
    float2 uv0 : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output;
	return output;
}