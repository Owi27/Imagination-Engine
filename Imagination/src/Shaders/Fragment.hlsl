float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

//struct VOut
//{
//    float4 pos : SV_Position;
//    float3 wPos : POSITION;
//    float3 nrm : NORMAL;
//    float2 uv : TEXCOORD0;
//    float4 tan : TANGENT;
//    float3 col : COLOR;

//    int idx : INDEX;
//};

//struct FOut
//{
//    float4 Albedo : SV_TARGET0;
//    float4 Normal : SV_TARGET1;
//    //float4 Albedo   : SV_TARGET2;
//};

//cbuffer GBufferPC
//{
//    matrix model;
//    uint materialIndex;
//};

//#define ALPHA_OPAQUE 0
//#define ALPHA_MASK   1
//#define ALPHA_BLEND  2

//struct Material
//{
//    float4 baseColorFactor;
//    float4 emissiveFactor;

//    int4 textureIndices0;
//    int4 textureIndices1;

//    float4 materialFactors;
//    float4 extraFactors;
//};

//StructuredBuffer<Material> materialInfo : register(t1, space0);

//Texture2D materialTextures[] : register(t0, space1);
//SamplerState materialSampler : register(s0, space1);

//FOut main(VOut input)
//{
//    FOut output;

//    Material m = materialInfo[materialIndex];
//    int baseColorTexture = m.textureIndices0.x;
//    int metallicRoughnessTexture = m.textureIndices0.y;
//    int emissiveTexture = m.textureIndices0.z;
//    int normalTexture = m.textureIndices0.w;
//    int occlusionTexture = m.textureIndices1.x;
//    int alphaMode = m.textureIndices1.y;
//    int doubleSided = m.textureIndices1.z;
//    float metallicFactor = m.materialFactors.x;
//    float roughnessFactor = m.materialFactors.y;
//    float alphaCutoff = m.materialFactors.z;
//    float normalTextureScale = m.materialFactors.w;
//    float occlusionTextureStrength = m.extraFactors.x;
    
//    float3 T = normalize(input.tan.xyz);
//    float3 N = normalize(input.nrm);
//    T = normalize(T - dot(T, N) * N); // Gram-Schmidt
//    float3 B = cross(N, T) * input.tan.w; // handedness
//    float3x3 TBN = float3x3(T, B, N);
    
//    float3 worldNormal = normalize(mul(normalize(input.nrm), TBN));

//    float4 albedo = m.baseColorFactor;

//    if (baseColorTexture > -1)
//    {
//        albedo *= materialTextures[baseColorTexture].Sample(materialSampler, input.uv);
//    }

//    if (alphaMode == ALPHA_MASK)
//    {
//        if (albedo.a < alphaCutoff)
//            discard;
//    }

//    if (alphaMode == ALPHA_BLEND)
//    {
//        // temporary: discard low-alpha pixels in deferred pass
//        if (albedo.a < 0.5f)
//            discard;
//    }

//    output.Albedo = float4(albedo.rgb, 1.0f);
//    output.Normal = float4(worldNormal * .5f + .5f, 1.f);
//    //output.Albedo = float4(albedo.rgb, 1.0f);

//    return output;
//}