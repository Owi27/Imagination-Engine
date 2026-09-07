Texture2D<float4> taaInput[5] : register(t2, space0);
RWTexture2D<float4> taaOutput : register(u3, space0);


[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint2 pixel = DTid.xy;
    uint width, height;
    taaOutput.GetDimensions(width, height);
    
    float2 uv = (float2(pixel) + .5f) / float2(width, height);
    
    float3 currColor = taaInput[0].Load(int3(pixel, 0)).rgb;
    float3 prevColor = taaInput[1].Load(int3(pixel, 0)).rgb;
    
    taaOutput[pixel] = float4(currColor * .1f + prevColor * .9f, 1.f);
}