#define LitScene 0
#define TAAHistory 1
#define Velocity 2
#define VHistory 3

[[vk::push_constant]]
struct TAAPC
{
    uint validHistory;
} pc;


Texture2D<float4> taaInput[4] : register(t2, space0);
RWTexture2D<float4> taaOutput : register(u3, space0);
SamplerState _sampler : register(s4, space0);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 pixel = DTid.xy;
    uint width, height;
    taaOutput.GetDimensions(width, height);
    
    float2 uv = (float2(pixel) + .5f) / float2(width, height);
    
    float2 currVelocityUV = taaInput[Velocity].Load(int3(pixel, 0)).rg;
    float2 reprojectedUV = uv - currVelocityUV;
    float3 currColor = taaInput[LitScene].Load(int3(pixel, 0)).rgb;
//    Load(int3(reprojectedUV, 0)).rgb;
    
    float3 prevColor = currColor;
    //valid history
    if (pc.validHistory == 1)
        prevColor = taaInput[TAAHistory].Sample(_sampler, reprojectedUV).rgb;
    
    float3 minColor = 9999.f, maxColor = -9999.f, currFrameBlurred = 0.f;
    
    //color clamping
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            float3 color = taaInput[LitScene].Load(int3(clamp(pixel + int2(x, y), int2(0, 0), int2(width - 1, height - 1)), 0.f));
            
            currFrameBlurred += color;
            minColor = min(minColor, color);
            maxColor = max(maxColor, color);
        }
    }
    
    currFrameBlurred *= .11111111111111111111111111111111f; ///= 9.f;
    
    float3 previousColorClamped = clamp(prevColor, minColor, maxColor);
    float3 accumulation = currColor * .1f + previousColorClamped * .9f;
    
    float velocityDisocclusion = 0.f;

    if (pc.validHistory == 1)
    {
        float2 previousVelocityUV = taaInput[VHistory].Sample(_sampler, reprojectedUV).rg;
        float velocityLength = length(previousVelocityUV - currVelocityUV);

        velocityDisocclusion = saturate((velocityLength - 0.001f) * 10.f);
    }
    
    taaOutput[pixel] = float4(lerp(accumulation, currFrameBlurred, velocityDisocclusion), 1.f);
}