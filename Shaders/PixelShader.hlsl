struct VertexOUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

//float4 main(VertexOUT input) : SV_TARGET
//{
//    return input.col;
//}

// Push constants (no descriptor set needed)
struct ParamsPC
{
    float uTime;
};
[[vk::push_constant]] ParamsPC gPC;

float hash(int2 p)
{
    int n = p.x * 3 + p.y * 113;
    n = (n << 13) ^ n;
    n = n * (n * n * 15731 + 789221) + 1376312589;
    // keep only lower 28 bits to match 0x0fffffff range used in original
    int m = (n & 0x0fffffff);
    return -1.0 + 2.0 * (float(m) / float(0x0fffffff));
}

float Noise(float2 p)
{
    int2 i = int2(floor(p));
    float2 f = frac(p);
    float2 u = f * f * (3.0 - 2.0 * f);

    float n00 = hash(i + int2(0, 0));
    float n10 = hash(i + int2(1, 0));
    float n01 = hash(i + int2(0, 1));
    float n11 = hash(i + int2(1, 1));

    float nx0 = lerp(n00, n10, u.x);
    float nx1 = lerp(n01, n11, u.x);
    return lerp(nx0, nx1, u.y);
}

float NoiseValue(float2 uv)
{
    float2 noisePos = float2(uv.x + gPC.uTime, uv.y);
    float2 noiseUV = noisePos * 2.5;
    float2x2 m = float2x2(1.6, 1.2, -1.2, 1.6);

    float height = 0.5000 * Noise(noiseUV);
    noiseUV = mul(m, noiseUV);
    height += 0.2500 * Noise(noiseUV);
    noiseUV = mul(m, noiseUV);
    height += 0.1250 * Noise(noiseUV);
    noiseUV = mul(m, noiseUV);
    height += 0.0625 * Noise(noiseUV);

    return 0.5 + 0.5 * height;
}

// PS input/output
struct PSIn
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

float4 main(PSIn input) : SV_TARGET
{
    float2 screenRes = float2(800, 600);
    float2 fragCoord = input.pos.xy; // pixel coords
    float2 uv = fragCoord / screenRes - 0.5; // [-0.5, 0.5]
    uv.x *= screenRes.x / screenRes.y; // fix aspect

    // Animated noise offset
    float rayOffset = NoiseValue(uv) * 1.1;
    float2 objectPosition = float2(1.5 + rayOffset, -1.5 + rayOffset);

    // Ray params
    float3 rp = float3(0.0, 0.0, -3.0);
    float3 rd = normalize(float3(uv * 4.0, 1.0));

    float3 col = float3(0.0, 0.0, 0.0);

    // Step forward along the ray; flash white when crossing shell
    [loop]
    for (int i = 0; i < 30; ++i)
    {
        rp += rd;
        float d = length(rp - float3(objectPosition, 0.0)) - 2.0;
        float f = frac(d);
        if (f < 0.01)
        {
            col = float3(1.0, 1.0, 1.0);
        }
    }

    return float4(col, 1.0);
}