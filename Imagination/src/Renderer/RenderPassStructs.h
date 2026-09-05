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
		mat4 viewProj, prevViewProj, jitteredViewProj;
	};

	struct GBufferMaterial
	{
		std::array<float, 4> baseColor;
		std::array<float, 3> emissive;
		float metallic, roughness, alphaCutoff;
		int alphaMode, doubleSided;
	};

	//lighting
	struct LightingPC
	{
		mat4 invViewProj;
		vec3 camPos;
		uint32_t width, height, pointLightCount;
	};

	//velocity
	struct Velocity
	{
		mat4 currViewProj, prevViewProj;
	};
}