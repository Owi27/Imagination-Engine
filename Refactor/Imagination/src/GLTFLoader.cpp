#include "pch.h"
#include "GLTFLoader.h"

void GLTFLoader::PullModelData()
{
	for (auto& node : _model->nodes)
	{
		if (node.mesh > -1)
		{
			auto& mesh = _model->meshes[node.mesh];

			for (auto& primitive : mesh.primitives)
			{
				tinygltf::Accessor& accessor = _model->accessors[primitive.attributes.find("POSITION")->second];
				tinygltf::BufferView& bufferView = _model->bufferViews[accessor.bufferView];
				tinygltf::Buffer& buffer = _model->buffers[bufferView.buffer];
			}
		}
	}
}

const uint8_t* GLTFLoader::GetAccessorDataPointer(const tinygltf::Accessor& accessor, size_t& strideBytesOut)
{
	tinygltf::BufferView& bufferView = _model->bufferViews[accessor.bufferView];
	tinygltf::Buffer& buffer = _model->buffers[bufferView.buffer];

	const uint8_t* data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

	strideBytesOut = accessor.ByteStride(bufferView);
	return data;
}

void GLTFLoader::GetIndex(std::vector<uint32_t>& pIndices, const tinygltf::Accessor& pAccessor, const tinygltf::BufferView& pBufferView, const tinygltf::Buffer& pBuffer)
{
	uint64_t stride = 0;
	const uint8_t* data = GetAccessorDataPointer(pAccessor, stride);

	switch (pAccessor.componentType)
	{
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
		{
			std::vector<uint8_t> indices(pAccessor.count);
			memcpy(indices.data(), data, pAccessor.count * stride);
			pIndices.insert(pIndices.end(), indices.begin(), indices.end());
		}
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		{
			std::vector<uint16_t> indices(pAccessor.count);
			memcpy(indices.data(), data, pAccessor.count * stride);
			pIndices.insert(pIndices.end(), indices.begin(), indices.end());
		}
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		{
			std::vector<uint32_t> indices(pAccessor.count);
			memcpy(indices.data(), data, pAccessor.count * stride);
			pIndices.insert(pIndices.end(), indices.begin(), indices.end());
		}
		break;
	default:
		// unsupported index type
		break;
	}
}

vec2 GLTFLoader::ReadVec2(int pAccessorIdx, uint32_t i)
{
	if (pAccessorIdx < 0) return { 0, 0 };

	const tinygltf::Accessor& accessor = _model->accessors[pAccessorIdx];
	uint64_t stride;
	const uint8_t* data = GetAccessorDataPointer(accessor, stride);
	const float* value = reinterpret_cast<const float*>(data + stride * i);

	//out = { value[0], value[1] };
	return { value[0], value[1] };
}

vec3 GLTFLoader::ReadVec3(int pAccessorIdx, uint32_t i)
{
	if (pAccessorIdx < 0) return { 0, 0, 0 };

	const tinygltf::Accessor& accessor = _model->accessors[pAccessorIdx];
	uint64_t stride;
	const uint8_t* data = GetAccessorDataPointer(accessor, stride);
	const float* value = reinterpret_cast<const float*>(data + stride * i);

	//out = { value[0], value[1], value[2] };
	return { value[0], value[1], value[2] };
}

vec4 GLTFLoader::ReadVec4(int pAccessorIdx, uint32_t i)
{
	if (pAccessorIdx < 0) return { 0, 0, 0, 0 };

	const tinygltf::Accessor& accessor = _model->accessors[pAccessorIdx];
	uint64_t stride;
	const uint8_t* data = GetAccessorDataPointer(accessor, stride);
	const float* value = reinterpret_cast<const float*>(data + stride * i);

	//out = { value[0], value[1], value[2], value[3]};
	return { value[0], value[1], value[2], value[3] };
}

glMesh GLTFLoader::LoadModel(const std::string& filepath)
{
	tinygltf::TinyGLTF gltfLoader;
	std::string error;
	std::string warning;

	//unsigned long long pos = filename.find_last_of('/');
	//std::string path = filename.substr(0, pos);

	bool fileLoaded = gltfLoader.LoadASCIIFromFile(_model.get(), &error, &warning, filepath);
	//tinygltf::Model& model = _models[id];

	if (!warning.empty()) std::cout << "Warning: " << warning << '\n';

	if (!error.empty()) std::cout << "Error: " << error << '\n';

	if (!fileLoaded) std::cout << "Failed to parse model\n";

	glMesh loadedMesh;

	if (_model->images.size() > 0) loadedMesh.textures = _model->images;

	uint32_t vertexCursor = 0, idxCursor = 0;

	int sceneIndex = _model->defaultScene >= 0 ? _model->defaultScene : 0;

	if (sceneIndex < 0) sceneIndex = 0;

	const tinygltf::Scene& scene = _model->scenes[sceneIndex];

	for (auto& nodeIdx : scene.nodes)
	{
		const tinygltf::Node& node = _model->nodes[nodeIdx];

		if (node.mesh < 0) continue;

		const tinygltf::Mesh& mesh = _model->meshes[node.mesh];
		
		for (auto& primitive : mesh.primitives)
		{
			auto iter = primitive.attributes.find("POSITION");

			int posAccessorIdx = iter->second;

			int nrmAccessorIdx = -1;
			int uvAccessorIdx = -1;
			int tanAccessorIdx = -1;

			auto nrmIter = primitive.attributes.find("NORMAL");
			if (nrmIter != primitive.attributes.end()) nrmAccessorIdx = nrmIter->second;
			auto uvIter = primitive.attributes.find("TEXCOORD_0");
			if (uvIter != primitive.attributes.end()) uvAccessorIdx = uvIter->second;
			auto tanIter = primitive.attributes.find("TANGENT");
			if (tanIter != primitive.attributes.end()) tanAccessorIdx = tanIter->second;

			const tinygltf::Accessor& posAccessor = _model->accessors[posAccessorIdx];
			uint32_t vertexCount = static_cast<uint32_t>(posAccessor.count);
			uint32_t vertexOffset = static_cast<uint32_t>(vertexCursor);

			//loadedMesh.vertices.resize(loadedMesh.vertices.size() + vertexCount);

			for (uint32_t i = 0; i < vertexCount; i++)
			{
				Vertex v
				{
					.pos = ReadVec3(posAccessorIdx, i),
					.nrm = ReadVec3(nrmAccessorIdx, i),
					.uv = ReadVec2(uvAccessorIdx, i),
					.tan = ReadVec4(tanAccessorIdx, i)
				};

				loadedMesh.vertices.push_back(v);
			}

			vertexCursor += vertexCount;

			uint32_t idxCount = 0;
			uint32_t firstIdx = 0;
			bool hasIndices = primitive.indices >= 0;

			if (primitive.indices >= 0)
			{
				const tinygltf::Accessor& idxAccessor = _model->accessors[primitive.indices];
				const tinygltf::BufferView& idxBufferView = _model->bufferViews[idxAccessor.bufferView];
				const tinygltf::Buffer& idxBuffer = _model->buffers[idxBufferView.buffer];

				idxCount = static_cast<uint32_t>(idxAccessor.count);
				firstIdx = idxCursor;

				// Resize index buffer and fill
				//loadedMesh.indices.resize(idxCount);

				GetIndex(loadedMesh.indices, idxAccessor, idxBufferView, idxBuffer);
			/*	for (uint32_t i = 0; i < idxCount; ++i)
				{
					uint32_t localIndex = GetIndex(idxAccessor, i);
					loadedMesh.indices[idxCursor + i] = localIndex + vertexOffset;
				}

				idxCursor += idxCount*/;
			}
			else
			{
				idxCount = vertexCount;
				firstIdx = idxCursor;

				loadedMesh.indices.resize(loadedMesh.indices.size() + idxCount);

				for (uint32_t i = 0; i < idxCount; ++i)
				{
					loadedMesh.indices[idxCursor + i] = vertexOffset + i;
				}

				idxCursor += idxCount;
			}

			DrawInfo drawInfo
			{
				.idxCount = idxCount,
				.firstIdx = firstIdx,
				.vertexOffset = vertexOffset,
				.firstInst = 0,
				.instCount = 1,
				.matIdx = primitive.material < 0 ? 0 : (uint32_t)primitive.material
			};

			const tinygltf::Material& mat = _model->materials[drawInfo.matIdx];

			Material m
			{
				.baseColorFactor = {(float)mat.pbrMetallicRoughness.baseColorFactor[0], (float)mat.pbrMetallicRoughness.baseColorFactor[1], (float)mat.pbrMetallicRoughness.baseColorFactor[2], (float)mat.pbrMetallicRoughness.baseColorFactor[3]},
				.baseColorTexture = mat.pbrMetallicRoughness.baseColorTexture.index,
				.metallicFactor = (float)mat.pbrMetallicRoughness.metallicFactor,
				.roughnessFactor = (float)mat.pbrMetallicRoughness.roughnessFactor,
				.metallicRoughnessTexture = mat.pbrMetallicRoughness.metallicRoughnessTexture.index,
				.emissiveTexture = mat.emissiveTexture.index,
				.emissiveFactor = { (float)mat.emissiveFactor[0], (float)mat.emissiveFactor[1], (float)mat.emissiveFactor[2]},
				.alphaMode = 0,
				.alphaCutoff = (float)mat.alphaCutoff,
				.doubleSided = 0,
				.normalTexture = mat.normalTexture.index,
				.normalTextureScale = (float)mat.normalTexture.scale,
				.occlusionTexture = mat.occlusionTexture.index,
				.occlusionTextureStrength = (float)mat.occlusionTexture.strength
			};

			loadedMesh.drawInfo.push_back(drawInfo);
			loadedMesh.materials.push_back(m);
		}
	}

	return loadedMesh;
}
