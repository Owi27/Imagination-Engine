////static const float2 positions[3] =
////{
////    float2(0.0, -0.5),
////    float2(0.5, 0.5),
////    float2(-0.5, 0.5)
////};
////static const float3 colors[3] =
////{
////    float3(204.f/255.f, 180.f/255.f, 184.f/255.f),
////    float3(1.f, 184.f/255f, 198f/255f),
////    float3(51f/255f, 49f/255f, 60f/255f)
////};

////struct VIn
////{
////    float2 pos : POSITION;
////    float3 col : COLOR;
////    float2 uv : TEXCOORD0
////};

////struct VOut
////{
////    float4 pos : SV_Position;
////    float3 col : COLOR;
////};

////VOut main(VIn input)
////{
////    VOut output;
////    output.pos = float4(input.pos, 0.0, 1.0);
////    output.col = input.col;
    
////    return output;
////}

////struct VOut
////{
////    float4 pos : SV_Position;
////    float3 col : COLOR;
////};

////float4 main(VOut input) : SV_Target
////{
////    return float4(input.col, 1.0);
////}

////Gbuffer vertex
//struct VIn
//{
//    float3 pos : POSITION0;
//    float3 nrm : NORMAL0;
//    float2 uv : TEXCOORD0;
//    float4 tan : TANGENT;
//    float3 col : COLOR;
//};

//struct VOut
//{
//    float4 pos : SV_Position;
//    float3 nrm : NORMAL0;
//    float2 uv : TEXCOORD0;
//    float3 tan : TANGENT;
//    float3 col : COLOR;
//    float3x3 TBN : TEXCOORD1;
//    int idx : INDEX;
//};

//struct UniformBuffer
//{
//    matrix model;
//    matrix view;
//    matrix proj;
//};
//ConstantBuffer<UniformBuffer> ubo : register(b0, space0);

//VOut main(VIn input, uint id : SV_InstanceID)
//{
//    VOut output;
//    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.pos, 1))));
//    output.uv = input.uv;
//    output.nrm = normalize(input.nrm);
//    output.tan = input.tan;
    
//    return output;
//}

////gbuffer frag
//struct VOut
//{
//    float4 pos : SV_Position;
//    float3 nrm : NORMAL0;
//    float2 uv : TEXCOORD0;
//    float3 tan : TANGENT;
//    float3 col : COLOR;
//    float3x3 TBN : TEXCOORD1;
//    int idx : INDEX;
//};

//struct FOut
//{
//    float4 Position : SV_TARGET0;
//    float4 Normal : SV_TARGET1;
//    float4 Albedo : SV_TARGET2;
//    float4 MProperties : SV_TARGET3;
//};

//VOut main(VOut input, uint id : SV_InstanceID)
//{
//    FOut output;
//    output.Position = input.pos;
//    output.Albedo = float4(0.f, 0.f, 1.f, 1.f);
    
//    return output;
//}