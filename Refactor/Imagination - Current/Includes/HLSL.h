#include "pch.h"

namespace Shaders
{
	std::string TriangleVertexShader =
		R"(struct VIn
{
    float2 pos : POSITION;
    float3 col : COLOR;
};

struct VOut
{
    float4 pos : SV_Position;
    float3 col : COLOR;
};

VOut main(VIn input)
{
    VOut output;
    output.pos = float4(input.pos, 0.0, 1.0);
    output.col = input.col;
    
    return output;
})";

    std::string TriangleFragmentShader =
        R"(struct VOut
{
    float4 pos : SV_Position;
    float3 col : COLOR;
};

float4 main(VOut input) : SV_Target
{
    return float4(input.col, 1.0);
})";
}