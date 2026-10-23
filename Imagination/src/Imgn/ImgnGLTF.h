#pragma once
#include "gltf/tiny_gltf.h"
#include "ImgnRenderer.h"
#include "Components/MeshComponent.h"
#include "Components/MaterialComponent.h"
#include <filesystem>
#include <utility>
#include "Skeleton.h"

namespace Imgn
{
	struct MeshData
	{
		std::string name;
		shared<Buffer> vertexBuffer, indexBuffer;
		std::vector<Primitive> primitives;
	};

	struct ImgnModel
	{
		std::vector<MeshData> meshes;
		std::vector<shared<Material>> materials;

		std::vector<Skeleton> skeletons;
	};

	class IMGN_API GLTFLoader
	{
		static inline unique<GLTFLoader> _instance;

		std::vector<uint32_t> LoadGLTFTextures(const tinygltf::Model& pModel, ImgnRenderer& pRenderer);
		std::vector<shared<Material>> LoadGLTFMaterials(const tinygltf::Model& pModel, const std::vector<uint32_t>& pTextures);
		std::vector<MeshData> LoadGLTFMeshes(const tinygltf::Model& pModel, ImgnRenderer& pRenderer);
		std::pair<std::vector<Vertex>, std::vector<uint32_t>> GetVertexData(const tinygltf::Model& pModel, const tinygltf::Primitive& pPrimitive);
		ImgnModel LoadModelImpl(const std::filesystem::path& pFile, ImgnRenderer& pRenderer);
		Skeleton LoadSkeleton(const tinygltf::Model& pModel, const tinygltf::Skin& pSkin);
		std::vector<int32_t> BuildNodeParents(const tinygltf::Model& pModel);

		static bool LoadFile(const std::filesystem::path& pFile, tinygltf::Model& pModel);

	public:
		/* Class Defaults */
		GLTFLoader()
		{

		}

		~GLTFLoader()
		{

		}

		inline static GLTFLoader& Get()
		{
			if (!_instance) _instance.reset(new GLTFLoader());
			return *_instance;
		}

		/* Class Functions */
		static ImgnModel LoadModel(const std::filesystem::path& pFile, ImgnRenderer& pRenderer);
		static std::vector<MeshData> LoadMeshes(const std::filesystem::path& pFile, ImgnRenderer& pRenderer);
	};


}