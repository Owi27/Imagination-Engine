#include "pch.hpp"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ImgnGLTF.h"

namespace Imgn
{
	std::vector<uint32_t> GLTFLoader::LoadGLTFTextures(const tinygltf::Model& pModel, Imgn::ImgnRenderer& pRenderer)
	{
		std::vector<uint32_t> textures; textures.reserve(pModel.images.size());

		for (const tinygltf::Image& image : pModel.images)
		{
			textures.push_back(pRenderer.CreateImage(image.width, image.height, image.image.data()));
		}

		return textures;
	}

	std::vector<uint32_t> GLTFLoader::LoadGLTFMaterials(const tinygltf::Model& pModel, const std::vector<uint32_t>& pTextures, Imgn::ImgnRenderer& pRenderer)
	{
		std::vector<uint32_t> materials; materials.reserve(pModel.materials.size());

		if (pModel.materials.empty())
		{
			materials.push_back(0);
			return materials;
		}

		auto GetTextureHandle = [&](int pTextureIndex) -> int32_t
			{
				if (pTextureIndex < 0 || pTextureIndex >= static_cast<int32_t>(pModel.textures.size())) return -1;

				const int32_t imageIndex = pModel.textures[pTextureIndex].source;

				if (imageIndex < 0 || imageIndex >= static_cast<int32_t>(pTextures.size())) return -1;

				return static_cast<int32_t>(pTextures[imageIndex]);
			};

		for (const tinygltf::Material& material : pModel.materials)
		{
			const tinygltf::PbrMetallicRoughness& pbr = material.pbrMetallicRoughness;

			Material m
			{
				.baseColorFactor =
				{
					pbr.baseColorFactor.size() == 4 ? static_cast<float>(pbr.baseColorFactor[0]) : 1.0f,
					pbr.baseColorFactor.size() == 4 ? static_cast<float>(pbr.baseColorFactor[1]) : 1.0f,
					pbr.baseColorFactor.size() == 4 ? static_cast<float>(pbr.baseColorFactor[2]) : 1.0f,
					pbr.baseColorFactor.size() == 4 ? static_cast<float>(pbr.baseColorFactor[3]) : 1.0f
				},

				.emissiveFactor =
				{
					material.emissiveFactor.size() == 3 ? static_cast<float>(material.emissiveFactor[0]) : 0.0f,
					material.emissiveFactor.size() == 3 ? static_cast<float>(material.emissiveFactor[1]) : 0.0f,
					material.emissiveFactor.size() == 3 ? static_cast<float>(material.emissiveFactor[2]) : 0.0f,
					0.0f
				},

				.textureIndices0 =
				{
					GetTextureHandle(pbr.baseColorTexture.index),
					GetTextureHandle(pbr.metallicRoughnessTexture.index),
					GetTextureHandle(material.emissiveTexture.index),
					GetTextureHandle(material.normalTexture.index)
				},

				.textureIndices1 =
				{
					GetTextureHandle(material.occlusionTexture.index),
					material.alphaMode == "BLEND" ? static_cast<int32_t>(ImgnAlphaMode::Blend) :
					material.alphaMode == "MASK" ? static_cast<int32_t>(ImgnAlphaMode::Mask) :
					static_cast<int32_t>(ImgnAlphaMode::Opaque),
					material.doubleSided ? 1 : 0,
					0
				},

				.materialFactors =
				{
					static_cast<float>(pbr.metallicFactor),
					static_cast<float>(pbr.roughnessFactor),
					static_cast<float>(material.alphaCutoff),
					static_cast<float>(material.normalTexture.scale)
				},

				.extraFactors =
				{
					static_cast<float>(material.occlusionTexture.strength),
					0.0f,
					0.0f,
					0.0f
				}
			};

			materials.push_back(pRenderer.AddMaterial(m));
		}

		return materials;
	}

	std::vector<uint32_t> GLTFLoader::LoadGLTFMeshes(const tinygltf::Model& pModel, Imgn::ImgnRenderer& pRenderer)
	{
		std::vector<uint32_t> meshes;

		for (const tinygltf::Mesh& mesh : pModel.meshes)
		{
			ImgnMesh m;
			m.name = mesh.name;

			std::vector<Vertex> vertices;// = vertexData.first;
			std::vector<uint32_t> indices;// = vertexData.second;

			for (const tinygltf::Primitive& primitive : mesh.primitives)
			{
				if (primitive.mode != TINYGLTF_MODE_TRIANGLES) continue;

				auto vertexData = GetVertexData(pModel, primitive);

				ImgnPrimitive prim
				{
					.name = mesh.name + "Primitive",
					.vertexOffset = static_cast<int>(vertices.size()),
					.firstIndex = static_cast<uint32_t>(indices.size()),
					.material = primitive.material > -1 ? static_cast<uint32_t>(primitive.material) : 0,
				};

				vertices.insert(vertices.end(), vertexData.first.begin(), vertexData.first.end());
				indices.insert(indices.end(), vertexData.second.begin(), vertexData.second.end());

				prim.indexCount = static_cast<uint32_t>(indices.size()) - prim.firstIndex;

				m.primitives.push_back(prim);
			}

			m.vertexBuffer = pRenderer.CreateVertexBuffer(vertices);
			m.indexBuffer = pRenderer.CreateIndexBuffer(indices);

			meshes.push_back(pRenderer.AddMesh(m));
		}

		return meshes;
	}

	void GLTFLoader::CreateMaterialBuffer(ImgnRenderer& pRenderer)
	{
		_outModel.materialBuffer = pRenderer.CreateMaterialBuffer(_outModel.materials);
		_outModel.materialBufferSize = static_cast<uint64_t>(_outModel.materials.size()) * sizeof(Material);
	}

	std::pair<std::vector<Vertex>, std::vector<uint32_t>> GLTFLoader::GetVertexData(const tinygltf::Model& pModel, const tinygltf::Primitive& pPrimitive)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		//pos
		const tinygltf::Accessor& posAccessor = pModel.accessors[pPrimitive.attributes.at("POSITION")];
		const tinygltf::BufferView& posBufferView = pModel.bufferViews[posAccessor.bufferView];
		const tinygltf::Buffer& posBuffer = pModel.buffers[posBufferView.buffer];
		const uint64_t posStride = posAccessor.ByteStride(posBufferView);

		const tinygltf::Accessor* nrmAcc = pPrimitive.attributes.contains("NORMAL") ? &pModel.accessors[pPrimitive.attributes.at("NORMAL")] : nullptr;
		const tinygltf::Accessor* uvAcc = pPrimitive.attributes.contains("TEXCOORD_0") ? &pModel.accessors[pPrimitive.attributes.at("TEXCOORD_0")] : nullptr;
		const tinygltf::Accessor* tanAcc = pPrimitive.attributes.contains("TANGENT") ? &pModel.accessors[pPrimitive.attributes.at("TANGENT")] : nullptr;
		const tinygltf::Accessor* jointAcc = pPrimitive.attributes.contains("JOINTS_0") ? &pModel.accessors[pPrimitive.attributes.at("JOINTS_0")] : nullptr;
		const tinygltf::Accessor* weightAcc = pPrimitive.attributes.contains("WEIGHTS_0") ? &pModel.accessors[pPrimitive.attributes.at("WEIGHTS_0")] : nullptr;

		for (size_t i = 0; i < posAccessor.count; i++)
		{
			Vertex v{};

			//pos
			const float* p = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset + (i * posStride)]);
			v.pos = { p[0], p[1], p[2] };

			//nrm
			if (nrmAcc)
			{
				const auto& view = pModel.bufferViews[nrmAcc->bufferView];
				const float* n = reinterpret_cast<const float*>(&pModel.buffers[view.buffer].data[view.byteOffset + nrmAcc->byteOffset + (i * nrmAcc->ByteStride(view))]);
				v.nrm = { n[0], n[1], n[2] };
			}

			//uv
			if (uvAcc)
			{
				const auto& view = pModel.bufferViews[uvAcc->bufferView];
				const float* u = reinterpret_cast<const float*>(&pModel.buffers[view.buffer].data[view.byteOffset + uvAcc->byteOffset + (i * uvAcc->ByteStride(view))]);
				v.uv0 = { u[0], u[1] };
			}

			//tan
			if (tanAcc)
			{
				const auto& view = pModel.bufferViews[tanAcc->bufferView];
				const float* t = reinterpret_cast<const float*>(&pModel.buffers[view.buffer].data[view.byteOffset + tanAcc->byteOffset + (i * tanAcc->ByteStride(view))]);
				v.tan = { t[0], t[1], t[2], t[3] };
			}

			v.clr = { 1.f, 1.f, 1.f };

			vertices.push_back(v);
		}

		//indices
		const auto& idxAcc = pModel.accessors[pPrimitive.indices];
		const auto& idxView = pModel.bufferViews[idxAcc.bufferView];
		const auto& idxBuf = pModel.buffers[idxView.buffer];
		const unsigned char* data = &idxBuf.data[idxView.byteOffset + idxAcc.byteOffset];

		for (size_t i = 0; i < idxAcc.count; i++)
		{
			uint32_t localIdx = 0;
			if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
				localIdx = reinterpret_cast<const uint16_t*>(data)[i];
			else if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
				localIdx = reinterpret_cast<const uint32_t*>(data)[i];
			else if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
				localIdx = reinterpret_cast<const uint8_t*>(data)[i];

			indices.push_back(localIdx);
		}

		return { vertices, indices };
	}

	ImgnModel GLTFLoader::LoadModelImpl(const std::filesystem::path& pFile, ImgnRenderer& pRenderer)
	{
		tinygltf::TinyGLTF loader;
		tinygltf::Model model;
		std::string error;
		std::string warning;

		bool loaded = false;

		if (pFile.extension() == ".glb") loaded = loader.LoadBinaryFromFile(&model, &error, &warning, pFile.string());
		else loaded = loader.LoadASCIIFromFile(&model, &error, &warning, pFile.string());

		if (!warning.empty())
		{
			std::cout << "Warn:  " << warning << '\n';
		}

		if (!error.empty())
		{
			std::cout << "Err:  " << error << '\n';
		}

		if (!loaded)
		{
			std::cout << "Failed to parse glTF: " << pFile << '\n';
			throw std::runtime_error("Failed to parse glTF: " + pFile.string() + "\n" + error);
		}

		_outModel.materials = LoadGLTFMaterials(model, LoadGLTFTextures(model, pRenderer), pRenderer);
		_outModel.meshes = LoadGLTFMeshes(model, pRenderer);
		CreateMaterialBuffer(pRenderer);

		return _outModel;
	}

	Skeleton GLTFLoader::LoadSkeleton(const tinygltf::Model& pModel, const tinygltf::Skin& pSkin)
	{
		Skeleton skeleton;
		skeleton.name = pSkin.name;
		skeleton.rootNode = pSkin.skeleton;
		skeleton.joints.resize(pSkin.joints.size());

		for (size_t i = 0; i < pSkin.joints.size(); i++)
		{
			int nodeIdx = pSkin.joints[i];
			const tinygltf::Node& node = pModel.nodes[nodeIdx];
			Joint& joint = skeleton.joints[i];
			joint.name = node.name;
			joint.nodeIdx = nodeIdx;
			skeleton.nodeToJoint[nodeIdx] = i;
		}

		auto nodeParents = BuildNodeParents(pModel);

		for (size_t i = 0; i < pSkin.joints.size(); i++)
		{
			Joint& joint = skeleton.joints[i];
			int32_t parentNode = nodeParents[joint.nodeIdx];
			while (parentNode != -1)
			{
				if (skeleton.nodeToJoint.contains(parentNode))
				{
					joint.parentJoint = skeleton.nodeToJoint[parentNode];
					break;
				}

				parentNode = nodeParents[parentNode];
			}
		}

		const tinygltf::Accessor& accessor = pModel.accessors[pSkin.inverseBindMatrices];
		const tinygltf::BufferView& view = pModel.bufferViews[accessor.bufferView];
		const tinygltf::Buffer& buffer = pModel.buffers[view.buffer];
		const unsigned char* data = buffer.data.data() + view.byteOffset + accessor.byteOffset;


		return skeleton;
	}

	std::vector<int32_t> GLTFLoader::BuildNodeParents(const tinygltf::Model& pModel)
	{
		std::vector<int32_t> parents(pModel.nodes.size(), -1);

		for (size_t i = 0; i < pModel.nodes.size(); i++)
		{
			const tinygltf::Node& node = pModel.nodes[i];

			for (int childrenIdx : node.children)
			{
				parents[childrenIdx] = static_cast<int32_t>(i);
			}
		}

		return parents;
	}

	ImgnModel GLTFLoader::LoadModel(const std::filesystem::path& pFile, Imgn::ImgnRenderer& pRenderer)
	{
		return _instance->LoadModelImpl(pFile, pRenderer);
	}
}