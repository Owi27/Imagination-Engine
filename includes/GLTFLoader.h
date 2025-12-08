#pragma once

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "gltf/tiny_gltf.h"
#include "Resource.hpp"

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
	int _pad = 0;
};

struct GeometryData
{
	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<vec2> texCoords;
	std::vector<vec4> tangents;
	std::vector<unsigned> indices;
};

class GLTFLoader
{
	std::unique_ptr<tinygltf::Model> _model;

public:

	std::unique_ptr<void*> vertexBuffer;

	GLTFLoader() /*Constructor*/
	{
	}

	~GLTFLoader() /*Destructor*/
	{
	}

	GLTFLoader(const GLTFLoader& pOther) /*Copy Constructor*/
	{
	}

	GLTFLoader& operator=(const GLTFLoader& pOther) /*Copy Assignment Operator*/
	{
		if (this != &pOther)
		{
		}

		return *this;
	}

	GLTFLoader(GLTFLoader&& pOther) noexcept /*Move Constructor*/
	{
	}

	GLTFLoader& operator=(GLTFLoader&& pOther) noexcept /*Move Assignment Operator*/
	{
		if (this != &pOther)
		{
		}

		return *this;
	}

	void LoadModel(const char* filepath);
};