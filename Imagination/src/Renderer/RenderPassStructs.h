#pragma once

namespace Imgn
{
	struct GBufferPC
	{
		mat4 model;
		mat4 prevModel;
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
	struct TAAPC
	{
		bool historyValid;
	};
}