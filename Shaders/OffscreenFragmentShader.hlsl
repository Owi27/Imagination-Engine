struct GBuffer
{
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 albedo : SV_Target2;
};

//pos
GBuffer main()
{
    GBuffer gBuffer;
    //gBuffer. pos = input.pos
    return gBuffer;
}