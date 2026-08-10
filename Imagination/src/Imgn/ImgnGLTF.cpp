#include "pch.hpp"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ImgnGLTF.h"

//void ImgnGLTF::LoadModel(const std::string& pFile)
//{
//	tinygltf::TinyGLTF loader;
//	tinygltf::Model model;
//	std::string error;
//	std::string warning;
//
//	bool r = loader.LoadASCIIFromFile(&model, &error, &warning, pFile);
//
//	if (!warning.empty())
//	{
//		printf("Warn: %s\n", warning.c_str());
//	}
//
//	if (!error.empty())
//	{
//		printf("Err: %s\n", error.c_str());
//	}
//
//	if (!r)
//	{
//		printf("Failed to parse glTF: %s\n", pFile.c_str());
//	}
//
//	model.materials[0].pbrMetallicRoughness.
//}
//
//std::vector<uint32_t> ImgnGLTF::LoadGLTFTextures(const tinygltf::Model& pModel)
//{
//}
//
//std::vector<uint32_t> ImgnGLTF::LoadGLTFMaterials(const tinygltf::Model& pModel)
//{
//	std::vector<uint32_t> materials(pModel.materials.size());
//
//	for (const tinygltf::Material& material : pModel.materials)
//	{
//		const tinygltf::PbrMetallicRoughness& pbr = material.pbrMetallicRoughness;
//
//		ImgnMaterial m
//		{
//			.baseColor = pbr.baseColorFactor.size() == 4 ? vec4{ static_cast<float>(pbr.baseColorFactor[0]), static_cast<float>(pbr.baseColorFactor[1]), static_cast<float>(pbr.baseColorFactor[2]), static_cast<float>(pbr.baseColorFactor[3]) } : vec4{1.f, 1.f, 1.f, 1.f},
//			.emissive = material.emissiveFactor.size() == 3 ? vec3{ static_cast<float>(material.emissiveFactor[0]), static_cast<float>(material.emissiveFactor[1]), static_cast<float>(material.emissiveFactor[2]) } : vec3{1.f, 1.f, 1.f},
//			.baseColorTexture = pbr.baseColorTexture.index > -1 ? pModel.textures[pbr.baseColorTexture.index].source : -1,
//			.normalTexture = material.normalTexture.index > -1 ? pModel.textures[material.normalTexture.index].source : -1,
//			.metallicRoughnessTexture = pbr.metallicRoughnessTexture.index > -1 ? pModel.textures[pbr.metallicRoughnessTexture.index].source : -1,
//			.occlusionTexture = material.occlusionTexture.index > -1 ? pModel.textures[material.occlusionTexture.index].source : -1,
//			.emissiveTexture = material.emissiveTexture.index > -1 ? pModel.textures[material.emissiveTexture.index].source : -1,
//			.metallic = pbr.metallicFactor,
//			.roughness = pbr.roughnessFactor,
//			.alphaCutoff = material.alphaCutoff,
//			.alphaMode = material.alphaMode == "BLEND" ? ImgnAlphaMode::Blend : material.alphaMode == "MASK" ? ImgnAlphaMode::Mask : ImgnAlphaMode::Opaque
//		};
//
//		//todo
//	}
//
//	return materials;
//}
//
//std::vector<uint32_t> ImgnGLTF::LoadGLTFMeshes(const tinygltf::Model& pModel)
//{
//	std::vector<uint32_t> meshes;
//
//	for (const tinygltf::Mesh& mesh : pModel.meshes)
//	{
//		for (const tinygltf::Primitive& primitive : mesh.primitives)
//		{
//			if (primitive.mode != TINYGLTF_MODE_TRIANGLES) continue;
//
//
//		}
//	}
//
//	return meshes;
//}
//
//void ImgnGLTF::LoadModel(const std::filesystem::path& pFile)
//{
//	model.materials[0].pbrMetallicRoughness.
//
//}

namespace Imgn
{
	std::vector<uint32_t> ImgnGLTF::LoadGLTFTextures(const tinygltf::Model& pModel, Imgn::ImgnRenderer& pRenderer)
	{
		std::vector<uint32_t> textures; textures.reserve(pModel.images.size());

		for (const tinygltf::Image& image : pModel.images)
		{
			textures.push_back(pRenderer.CreateImage(image.width, image.height, image.image.data()));
		}

		return textures;
	}

	std::vector<uint32_t> ImgnGLTF::LoadGLTFMaterials(const tinygltf::Model& pModel, const std::vector<uint32_t>& pTextures, Imgn::ImgnRenderer& pRenderer)
	{
		std::vector<uint32_t> materials; materials.reserve(pModel.materials.size());

		if (pModel.materials.empty())
		{
			Material defaultMaterial
			{
				.baseColorFactor = { 1.f, 1.f, 1.f, 1.f },
				.emissiveFactor = { 0.f, 0.f, 0.f, 0.f },

				.textureIndices0 = { -1, -1, -1, -1 },

				.textureIndices1 =
				{
					-1,
					static_cast<int32_t>(ImgnAlphaMode::Opaque),
					0,
					0
				},

				.materialFactors = { 1.f, 1.f, 0.5f, 1.f },
				.extraFactors = { 1.f, 0.f, 0.f, 0.f }
			};

			materials.push_back(pRenderer.AddMaterial(defaultMaterial));
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

	std::vector<uint32_t> ImgnGLTF::LoadGLTFMeshes(const tinygltf::Model& pModel, Imgn::ImgnRenderer& pRenderer)
	{
		std::vector<uint32_t> meshes;
		//std::vector<Vertex> vertices;// = vertexData.first;
		//std::vector<uint32_t> indices;// = vertexData.second;

		for (const tinygltf::Mesh& mesh : pModel.meshes)
		{
			ImgnMesh m;

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
					.material = primitive.material > -1 ? static_cast<uint32_t>(primitive.material) : -1,
				};

				vertices.insert(vertices.end(), vertexData.first.begin(), vertexData.first.end());
				indices.insert(indices.end(), vertexData.second.begin(), vertexData.second.end());

				prim.indexCount = static_cast<uint32_t>(indices.size()) - prim.firstIndex;

				m.primitives.push_back(prim);
			}

			//ImgnBufferDesc vertexBufferDesc
			//{
			//	.name = mesh.name + "VertexBuffer",
			//	.size = sizeof(Vertex) * vertices.size(),
			//	.data = vertices.data(),
			//	.usage = ImgnBufferUsage::Vertex,
			//};

			//ImgnBufferDesc indexBufferDesc
			//{
			//	.name = mesh.name + "IndexBuffer",
			//	.size = sizeof(uint32_t) * indices.size(),
			//	.data = indices.data(),
			//	.usage = ImgnBufferUsage::Index,
			//};

			m.vertexBuffer = pRenderer.CreateVertexBuffer(vertices);
			m.indexBuffer = pRenderer.CreateIndexBuffer(indices);

			//m.vertexBuffer = pRenderer.CreateBuffer(vertexBufferDesc);
			//m.indexBuffer = pRenderer.CreateBuffer(indexBufferDesc);

			meshes.push_back(pRenderer.AddMesh(m));
		}

		return meshes;
	}

	void ImgnGLTF::CreateMaterialBuffer(const std::vector<uint32_t>& pMaterials, ImgnRenderer& pRenderer, ImgnModel& pModel)
	{
		pModel.materialBuffer = pRenderer.CreateMaterialBuffer(pMaterials);
		pModel.materialBufferSize = static_cast<uint64_t>(pModel.materials.size()) * sizeof(Material);
	}

	std::pair<std::vector<Vertex>, std::vector<uint32_t>> ImgnGLTF::GetVertexData(const tinygltf::Model& pModel, const tinygltf::Primitive& pPrimitive)
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

	ImgnModel ImgnGLTF::LoadModel(const std::filesystem::path& pFile, Imgn::ImgnRenderer& pRenderer)
	{
		ImgnModel out;

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

		out.materials = LoadGLTFMaterials(model, LoadGLTFTextures(model, pRenderer), pRenderer);
		out.meshes = LoadGLTFMeshes(model, pRenderer);
		CreateMaterialBuffer(out.materials, pRenderer, out);

		return out;
	}
}