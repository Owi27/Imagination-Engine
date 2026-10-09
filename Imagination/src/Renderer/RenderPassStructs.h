#pragma once

namespace Imgn
{
	struct GBufferPC
	{
		mat4 model, prevModel;
		uint32_t materialIndex = 0xFFFFFFFF, entityIDLow = 0, entityIDHigh = 0;
		uint32_t padding = 0;
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
		uint32_t historyValid;
	};

	//shadow
	struct ShadowPC
	{
		mat4 model;
	};

	struct ShadowUBO
	{
		mat4 viewProj;

		vec3 lightPosition;
		float farPlane;
	};
}