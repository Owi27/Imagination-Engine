#pragma once
#include "gltf/tiny_gltf.h"
#include "ImgnRenderer.h"
#include <filesystem>
#include <utility>

//enum class AlphaMode : uint32_t
//{
//	Opaque,
//	Mask,
//	Blend
//};

/*struct PbrMetallicRoughness {
  std::vector<double> baseColorFactor{1.0, 1.0, 1.0, 1.0};  // len = 4. default [1,1,1,1]
  TextureInfo baseColorTexture;
  double metallicFactor{1.0};   // default 1
  double roughnessFactor{1.0};  // default 1
  TextureInfo metallicRoughnessTexture;

  Value extras;
  ExtensionMap extensions;

  // Filled when SetStoreOriginalJSONForExtrasAndExtensions is enabled.
  std::string extras_json_string;
  std::string extensions_json_string;

  PbrMetallicRoughness() = default;
  DEFAULT_METHODS(PbrMetallicRoughness)

  bool operator==(const PbrMetallicRoughness &) const;
};
*/

//struct ImgnMaterial
//{
//	vec4 baseColor = { 1.f, 1.f, 1.f, 1.f }, emissive = { 0.f, 0.f, 0.f, 0.f };
//	int baseColorTexture = -1, normalTexture = -1, metallicRoughnessTexture = -1, occlusionTexture = -1, emissiveTexture = -1;
//	float metallic = 1.f, roughness = 1.f, alphaCutoff = .5f;
//	AlphaMode alphaMode = AlphaMode::Opaque;
//};


class ImgnGLTF
{

	std::vector<uint32_t> LoadGLTFTextures(const tinygltf::Model& pModel, ImgnRenderer& pRenderer);
	std::vector<uint32_t> LoadGLTFMaterials(const tinygltf::Model& pModel, ImgnRenderer& pRenderer);
	std::vector<uint32_t> LoadGLTFMeshes(const tinygltf::Model& pModel, ImgnRenderer& pRenderer);
	std::pair<std::vector<Vertex>, std::vector<uint32_t>> GetVertexData(const tinygltf::Model& pModel, const tinygltf::Primitive& pPrimitive);

public:
	/* Class Defaults */
	ImgnGLTF()
	{

	}

	~ImgnGLTF()
	{

	}

	/* Class Functions */
	void LoadModel(const std::filesystem::path& pFile, ImgnRenderer& pRenderer);
};