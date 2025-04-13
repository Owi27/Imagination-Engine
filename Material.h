#pragma once

struct Material
{
	vec4 baseColorFactor = { 1.f, 1.f, 1.f, 1.f };
	int baseColorTexture = -1;
	float metallicFactor = 1.f;
	float roughnessFactor = 1.f;
	int metallicRoughnessTexture = -1;
	int emissiveTexture = -1;
	vec3 emissiveFactor = { 0, 0, 0 };
	int alphaMode = 0;
	float alphaCutoff = 0.5f;
	int doubleSided = 0;
	int normalTexture = -1;
	float normalTextureScale = 1.f;
	int occlusionTexture = -1;
	float occlusionTextureStrength = 1.f;
};

