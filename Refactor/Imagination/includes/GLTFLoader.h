#pragma once
//#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "gltf/tiny_gltf.h"
#include "Structs.h"

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

struct Vertex
{
	vec3 pos, nrm;
	vec2 uv;
	vec4 tan;
};

struct DrawInfo
{
	uint32_t idxCount, firstIdx, vertexOffset, firstInst, instCount, matIdx;
};

struct glMesh
{
	mat4 world;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<DrawInfo> drawInfo;
	std::vector<tinygltf::Image> textures;
	std::vector<Material> materials;
};

struct GeometryData
{
	int vertexCount, instanceCount, firstVertex, firstInstance, indexCount, vertexOffset;
};

class GLTFLoader
{
	std::unique_ptr<tinygltf::Model> _model;
	std::vector<vec3> _positions;
	std::vector<vec3> _normals;
	std::vector<vec2> _texCoords;
	std::vector<vec4> _tangents;
	std::vector<unsigned> _indices;

	void PullModelData();
	const uint8_t* GetAccessorDataPointer(const tinygltf::Accessor& accessor, size_t& strideBytesOut);
	void GetIndex(std::vector<uint32_t>& pIndices, const tinygltf::Accessor& pAccessor, const tinygltf::BufferView& pBufferView, const tinygltf::Buffer& pBuffer);
	vec2 ReadVec2(int pAccessorIdx, uint32_t i);
	vec3 ReadVec3(int pAccessorIdx, uint32_t i);
	vec4 ReadVec4(int pAccessorIdx, uint32_t i);

public:

	mat4 world;
	std::unique_ptr<void*> vertexBuffer;

	GLTFLoader() /*Constructor*/
	{
		_model = std::make_unique<tinygltf::Model>();
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
	
	glMesh LoadModel(const std::string& filepath);
};