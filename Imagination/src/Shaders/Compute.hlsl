#define LitScene 0
#define TAAHistory 1
#define Velocity 2
#define VelocityHistory 3

// Velocity rejection: UV-space scale for |prevVel - currVel| -> disocclusion [0,1].
// k=32 saturates at ~3% UV delta (Elo-style ghosting control).
static const float kVelocityDisocclusionScale = 32.0f;

[[vk::push_constant]]
struct TAAPC
{
    bool validHistory;
} pc;


Texture2D<float4> taaInput[4] : register(t2, space0);
RWTexture2D<float4> taaOutput : register(u3, space0);
SamplerState _sampler : register(s4, space0);

// 9-tap Catmull-Rom via bilinear-optimized fetches (MJP / UE4 style).
// Sharper history reconstruction than a single bilinear Sample.
float3 SampleHistoryCatmullRom(Texture2D<float4> tex, SamplerState samp, float2 uv, float2 texSize)
{
    float2 samplePos = uv * texSize;
    float2 texPos1 = floor(samplePos - 0.5f) + 0.5f;
    float2 f = samplePos - texPos1;

    float2 w0 = f * (-0.5f + f * (1.0f - 0.5f * f));
    float2 w1 = 1.0f + f * f * (-2.5f + 1.5f * f);
    float2 w2 = f * (0.5f + f * (2.0f - 1.5f * f));
    float2 w3 = f * f * (-0.5f + 0.5f * f);

    float2 w12 = w1 + w2;
    float2 offset12 = w2 / (w12 + 1e-5f);

    float2 texPos0 = texPos1 - 1.0f;
    float2 texPos3 = texPos1 + 2.0f;
    float2 texPos12 = texPos1 + offset12;

    texPos0 /= texSize;
    texPos3 /= texSize;
    texPos12 /= texSize;

    float3 result = 0.0f.xxx;
    result += tex.SampleLevel(samp, float2(texPos0.x,  texPos0.y),  0).rgb * (w0.x  * w0.y);
    result += tex.SampleLevel(samp, float2(texPos12.x, texPos0.y),  0).rgb * (w12.x * w0.y);
    result += tex.SampleLevel(samp, float2(texPos3.x,  texPos0.y),  0).rgb * (w3.x  * w0.y);

    result += tex.SampleLevel(samp, float2(texPos0.x,  texPos12.y), 0).rgb * (w0.x  * w12.y);
    result += tex.SampleLevel(samp, float2(texPos12.x, texPos12.y), 0).rgb * (w12.x * w12.y);
    result += tex.SampleLevel(samp, float2(texPos3.x,  texPos12.y), 0).rgb * (w3.x  * w12.y);

    result += tex.SampleLevel(samp, float2(texPos0.x,  texPos3.y),  0).rgb * (w0.x  * w3.y);
    result += tex.SampleLevel(samp, float2(texPos12.x, texPos3.y),  0).rgb * (w12.x * w3.y);
    result += tex.SampleLevel(samp, float2(texPos3.x,  texPos3.y),  0).rgb * (w3.x  * w3.y);

    return result;
}

// 3x3 max-magnitude velocity dilation (cheap silhouette / edge help).
float2 DilateVelocityMaxMagnitude(uint2 pixel, uint width, uint height)
{
    float2 bestVel = 0.0f.xx;
    float bestMag2 = -1.0f;
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            int2 p = clamp(int2(pixel) + int2(x, y), int2(0, 0), int2(width - 1, height - 1));
            float2 v = taaInput[Velocity].Load(int3(p, 0)).rg;
            float mag2 = dot(v, v);
            if (mag2 > bestMag2)
            {
                bestMag2 = mag2;
                bestVel = v;
            }
        }
    }
    return bestVel;
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 pixel = DTid.xy;
    uint width, height;
    taaOutput.GetDimensions(width, height);
    
    float2 uv = (float2(pixel) + .5f) / float2(width, height);

    // Dilate current velocity before reprojection (max-magnitude 3x3).
    float2 dilatedVel = DilateVelocityMaxMagnitude(pixel, width, height);
    float2 reprojectedUV = uv - dilatedVel;
    float3 currColor = taaInput[LitScene].Load(int3(pixel, 0)).rgb;
    
    float3 prevColor = currColor;
    //valid history — Catmull-Rom history sample (keep RGB AABB clamp below)
    if (pc.validHistory)
        prevColor = SampleHistoryCatmullRom(taaInput[TAAHistory], _sampler, reprojectedUV, float2(width, height));
    
    float3 minColor = 9999.f, maxColor = -9999.f;
    float3 neighborhoodSum = 0.0f.xxx;
    
    //color clamping (RGB AABB — YCoCg later) + 3x3 neighborhood blur accum
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            float3 color = taaInput[LitScene].Load(int3(clamp(pixel + int2(x, y), int2(0, 0), int2(width - 1, height - 1)), 0)).rgb;
            minColor = min(minColor, color);
            maxColor = max(maxColor, color);
            neighborhoodSum += color;
        }
    }
    float3 neighborhoodBlur = neighborhoodSum / 9.0f;
    
    float3 previousColorClamped = clamp(prevColor, minColor, maxColor);

    // Velocity rejection / disocclusion: compare previous-frame velocity at
    // reprojected UV against dilated current velocity.
    float disocclusion = 0.0f;
    if (pc.validHistory)
    {
        float2 prevVel = taaInput[VelocityHistory].SampleLevel(_sampler, reprojectedUV, 0).rg;
        disocclusion = saturate(kVelocityDisocclusionScale * length(prevVel - dilatedVel));
    }

    // Raise current weight with disocclusion; high-disocclusion path leans on
    // neighborhood-blurred current to reduce ghosting.
    float currWeight = lerp(0.1f, 1.0f, disocclusion);
    float3 currResolved = lerp(currColor, neighborhoodBlur, disocclusion);
    
    taaOutput[pixel] = float4(currResolved * currWeight + previousColorClamped * (1.0f - currWeight), 1.f);
}
