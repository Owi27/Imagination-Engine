#pragma once

namespace Imgn
{
	struct GBufferPC
	{
		mat4 model;
		uint32_t materialIndex = 0xFFFFFFFF;
		uint32_t _pad0 = 0, _pad1 = 0, _pad2 = 0;
		// float3x4 previous world matrix (fits push constants in 128 bytes with model+pads).
		std::array<float, 12> prevModel{};
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
	struct TAAPC
	{
		bool historyValid;
	};
}
