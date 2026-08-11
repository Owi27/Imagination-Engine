#pragma once

namespace Imgn
{
	struct GBufferPC
	{
		std::array<float, 16> model;
		uint32_t materialIndex = 0xFFFFFFFF;
	};

	struct GBufferUBO
	{
		std::array<float, 16> viewProj;
	};

	struct GBufferMaterial
	{
		std::array<float, 4> baseColor;
		std::array<float, 3> emissive;
		float metallic, roughness, alphaCutoff;
		int alphaMode, doubleSided;
	};
}