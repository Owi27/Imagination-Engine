#define TAAResolved 0
#define TAAHistory 1
#define Velocity 2

Texture2D<float4> taaInput[3] : register(t2, space0);
RWTexture2D<float4> taaOutput : register(u3, space0);
SamplerState _sampler : register(s4, space0);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 pixel = DTid.xy;
    uint width, height;
    taaOutput.GetDimensions(width, height);
    
    float2 uv = (float2(pixel) + .5f) / float2(width, height);
    
    float2 reprojectedUV = uv - float2(taaInput[Velocity].Load(int3(pixel, 0)).rg);
    float3 currColor = taaInput[TAAResolved].Load(int3(pixel, 0)).rgb;
//    Load(int3(reprojectedUV, 0)).rgb;
    
    float3 prevColor = currColor;
    //valid history
    if (all(reprojectedUV >= .0f) && all(reprojectedUV <= 1.f)) prevColor = taaInput[TAAHistory].SampleLevel(_sampler, reprojectedUV, 0).rgb;
    
    taaOutput[pixel] = float4(currColor * .1f + prevColor * .9f, 1.f);
}