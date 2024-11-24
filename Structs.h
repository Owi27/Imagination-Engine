#pragma once

struct Buffer
{
	VkBuffer buffer;
	VkDeviceMemory memory;
};

struct Texture
{
	Buffer texBuffer;
	VkImage texImage;
	VkImageView texImageView;
	VkSampler texSampler;
};

struct Matrices
{
	mat4 prevWorldViewProjection, currWorldViewProjection;
	float deltaTime;
};

template <typename T>
inline std::vector<T> GetVector(const tinygltf::Value& value)
{
	std::vector<T> result{ 0 };
	if (!value.IsArray())
		return result;
	result.resize(value.ArrayLen());

	for (int i = 0; i < value.ArrayLen(); i++)
	{
		result[i] = static_cast<T>(value.Get(i).IsNumber() ? value.Get(i).Get<double>() : value.Get(i).Get<int>());
	}

	return result;
}

inline void GetFloat(const tinygltf::Value& value, const std::string& name, float& val)
{
	if (value.Has(name))
	{
		val = static_cast<float>(value.Get(name).Get<double>());
	}
}

inline void GetInt(const tinygltf::Value& value, const std::string& name, int& val)
{
	if (value.Has(name))
	{
		val = value.Get(name).Get<int>();
	}
}

inline void GetVec2(const tinygltf::Value& value, const std::string& name, vec2& val)
{
	if (value.Has(name))
	{
		auto s = GetVector<float>(value.Get(name));
		val = { s[0], s[1] };
	}
}

inline void GetVec3(const tinygltf::Value& value, const std::string& name, vec3& val)
{
	if (value.Has(name))
	{
		auto s = GetVector<float>(value.Get(name));
		val = { s[0], s[1], s[2] };
	}
}

inline void GetVec4(const tinygltf::Value& value, const std::string& name, vec4& val)
{
	if (value.Has(name))
	{
		auto s = GetVector<float>(value.Get(name));
		val = { s[0], s[1], s[2], s[3] };
	}
}

inline void GetTexId(const tinygltf::Value& value, const std::string& name, int& val)
{
	if (value.Has(name))
	{
		val = value.Get(name).Get("index").Get<int>();
	}
}

template <uint64_t BATCHSIZE = 128>
inline void ParallelBatches(uint64_t numItems, std::function<void(uint64_t)> fn, uint32_t numThreads)
{
	if (numThreads <= 1 || numItems < numThreads || numItems < BATCHSIZE)
	{
		for (uint64_t idx = 0; idx < numItems; idx++)
		{
			fn(idx);
		}
	}
	else
	{
		std::atomic_uint64_t counter = 0;

		auto worker = [&]() {
			uint64_t idx;
			while ((idx = counter.fetch_add(BATCHSIZE)) < numItems)
			{
				uint64_t last = std::min(numItems, idx + BATCHSIZE);
				for (uint64_t i = idx; i < last; i++)
				{
					fn(i);
				}
			}
			};

		std::vector<std::thread> threads(numThreads);
		for (uint32_t i = 0; i < numThreads; i++)
		{
			threads[i] = std::thread(worker);
		}

		for (uint32_t i = 0; i < numThreads; i++)
		{
			threads[i].join();
		}
	}
}

struct BoundingBox
{
	BoundingBox() = default;
	BoundingBox(vec3 min, vec3 max)
	{
		_min = min;
		_max = max;
	}
	BoundingBox(const std::vector<vec3>& corners)
	{
		for (auto& c : corners)
		{
			Insert(c);
		}
	}

	void Insert(const vec3& v)
	{
		_min = { std::min(_min.x, v.x), std::min(_min.y, v.y), std::min(_min.z, v.z) };
		_max = { std::max(_max.x, v.x), std::max(_max.y, v.y), std::max(_max.z, v.z) };
	}

	void Insert(const BoundingBox& b)
	{
		Insert(b._min);
		Insert(b._max);
	}

	inline BoundingBox& operator+=(float v)
	{
		_min = { _min.x - v, _min.y - v, _min.z - v };
		_max = { _max.x - v, _max.y - v, _max.z - v };

		return *this;
	}

	inline bool IsEmpty() const
	{
		return (_min.x == FLT_MAX && _min.y == FLT_MAX && _min.z == FLT_MAX) || (_max.x == -FLT_MAX && _max.y == -FLT_MAX && _max.z == -FLT_MAX);
	}

	inline unsigned int Rank() const
	{
		unsigned int result = 0;
		result += _min.x < _max.x;
		result += _min.y < _max.y;
		result += _min.z < _max.z;
		return result;
	}

	inline bool isPoint() const { return _min.x == _max.x && _min.y == _max.y && _min.z == _max.z; }
	inline bool isLine() const { return Rank() == 1u; }
	inline bool isPlane() const { return Rank() == 2u; }
	inline bool isVolume() const { return Rank() == 3u; }
	inline vec3 Min() { return _min; }
	inline vec3 Max() { return _max; }
	inline vec3 Extents()
	{
		vec3 v;
		GVector2D::Subtract3F(_max, _min, v);
		return v;
	}
	inline vec3 Center()
	{
		vec3 v;
		GVector2D::Add3F(_min, _max, v);
		GVector2D::Scale3F(v, .5f, v);
		return v;
	}
	inline float Radius()
	{
		float f;
		vec3 v;
		GVector2D::Subtract3F(_max, _min, v);
		GVector2D::Magnitude3F(v, f);

		return f * .5f;
	}

	BoundingBox Transform(mat4 mat)
	{
		auto r = mat.row4;
		const float epsilon = 1e-6f;
		assert(fabs(r.x) < epsilon && fabs(r.y) < epsilon && fabs(r.z) < epsilon && fabs(r.w - 1.0f) < epsilon);

		std::vector<vec3> corners(8);
		vec4 v[8];

		GMatrix::VectorXMatrixF(mat, vec4{ _min.x, _min.y, _min.z, 1.f }, v[0]);
		GMatrix::VectorXMatrixF(mat, vec4{ _min.x, _min.y, _max.z, 1.f }, v[1]);
		GMatrix::VectorXMatrixF(mat, vec4{ _min.x, _max.y, _min.z, 1.f }, v[2]);
		GMatrix::VectorXMatrixF(mat, vec4{ _min.x, _max.y, _max.z, 1.f }, v[3]);
		GMatrix::VectorXMatrixF(mat, vec4{ _max.x, _min.y, _min.z, 1.f }, v[4]);
		GMatrix::VectorXMatrixF(mat, vec4{ _max.x, _min.y, _max.z, 1.f }, v[5]);
		GMatrix::VectorXMatrixF(mat, vec4{ _max.x, _max.y, _min.z, 1.f }, v[6]);
		GMatrix::VectorXMatrixF(mat, vec4{ _max.x, _max.y, _max.z, 1.f }, v[7]);

		corners[0] = { v[0].x, v[0].y, v[0].z };
		corners[1] = { v[1].x, v[1].y, v[1].z };
		corners[2] = { v[2].x, v[2].y, v[2].z };
		corners[3] = { v[3].x, v[3].y, v[3].z };
		corners[4] = { v[4].x, v[4].y, v[4].z };
		corners[5] = { v[5].x, v[5].y, v[5].z };
		corners[6] = { v[6].x, v[6].y, v[6].z };
		corners[7] = { v[7].x, v[7].y, v[7].z };

		BoundingBox result(corners);
		return result;
	}

private:
	vec3 _min = { FLT_MAX, FLT_MAX, FLT_MAX };
	vec3 _max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
};

struct KHR_Materials_PBRSpecularGlossiness
{
	vec4 diffuseFactor = { 1.f, 1.f, 1.f, 1.f };
	int diffuseTexture = -1;
	vec3 specularFactor = { 1.f, 1.f, 1.f };
	float glossinessFactor = 1.f;
	int specularGlossinessTexture = -1;
};

struct KHR_Materials_Specular
{
	float specularFactor = 1.f;
	int specularTexture = -1;
	vec3 specularColorFactor = { 1.f, 1.f, 1.f };
	int specularColorTexture = -1;
};

struct KHR_Texture_Transform
{
	vec2 offset = { 0.f, 0.f };
	float rotation = 0.f;
	vec2 scale = { 1.f, 1.f };
	int texCoord = 0;
	mat3 uvTransform = GW::MATH2D::GIdentityMatrix3F;  // Computed transform of offset, rotation, scale
};

struct KHR_Materials_Clearcoat
{
	float factor = 0.f;
	int texture = -1;
	float roughnessFactor = 0.f;
	int roughnessTexture = -1;
	int normalTexture = -1;
};

struct KHR_Materials_Sheen
{
	vec3 colorFactor = { 0.f, 0.f, 0.f };
	int colorTexture = -1;
	float roughnessFactor = 0.f;
	int roughnessTexture = -1;
};

struct KHR_Materials_Transmission
{
	float factor = 0.f;
	int texture = -1;
};

struct KHR_Materials_Unlit
{
	int active = 0;
};

struct KHR_Materials_Anisotropy
{
	float factor{ 0.f };
	vec3 direction = { 1.f, 0.f, 0.f };
	int texture = -1;
};

struct KHR_Materials_IOR
{
	float ior = 1.5f;
};

struct KHR_Materials_Volume
{
	float thicknessFactor = 0.f;
	int thicknessTexture = -1;
	float attenuationDistance = FLT_MAX;
	vec3 attenuationColor = { 1.f, 1.f, 1.f };
};

struct KHR_Texture_Basisu
{
	int source = -1;
};

struct KHR_Materials_Displacement
{
	float displacementGeometryFactor = 1.f;
	float displacementGeometryOffset = 0.f;
	int displacementGeometryTexture = -1;
};

struct KHR_Materials_EmissiveStrength
{
	float emissiveStrength = 1.f;
};

struct GLTFMaterial
{
	//pbrMetallicRoughness
	vec4 baseColorFactor = { 1.f, 1.f, 1.f, 1.f };
	int baseColorTexture = -1;
	float metallicFactor = 1.f;
	float roughnessFactor = 1.f;
	int metallicRoughnessTexture = -1;
	int emissiveTexture = -1;
	vec3 emissiveFactor = { 0.f, 0.f, 0.f };
	int alphaMode = 0;
	float alphaCutoff = .5f;
	int doubleSided = 0;
	int normalTexture = -1;
	float normalTextureScale = 1.f;
	int occlusionTexture = -1;
	float occlusionTextureStrength = 1;

	//extensions
	KHR_Materials_PBRSpecularGlossiness specularGlossiness;
	KHR_Materials_Specular specular;
	KHR_Texture_Transform textureTransform;
	KHR_Materials_Clearcoat clearcoat;
	KHR_Materials_Sheen sheen;
	KHR_Materials_Transmission transmission;
	KHR_Materials_Unlit unlit;
	KHR_Materials_Anisotropy anisotropy;
	KHR_Materials_IOR ior;
	KHR_Materials_Volume volume;
	KHR_Materials_Displacement displacement;
	KHR_Materials_EmissiveStrength emissiveStrength;

	//tiny ref
	const tinygltf::Material* material = nullptr;
};

struct GLTFNode
{
	mat4 worldMatrix = GW::MATH::GIdentityMatrixF;
	int primMesh = 0;

	//tiny ref
	const tinygltf::Node* node = nullptr;
};

struct GLTFPrimMesh
{
	unsigned int firstIndex = 0;
	unsigned int indexCount = 0;
	unsigned int vertexOffset = 0;
	unsigned int vertexCount = 0;
	int materialIndex = 0;

	vec3 posMin = { 0, 0, 0 }, posMax = { 0, 0, 0 };
	std::string name;

	//tiny ref
	const tinygltf::Mesh* mesh = nullptr;
	const tinygltf::Primitive* prim = nullptr;
};

struct GLTFCamera
{
	mat4 worldMatrix = GW::MATH::GIdentityMatrixF;
	vec3 eye = { 0, 0, 0 }, center = { 0, 0, 0 }, up = { 0, 1, 0 };

	tinygltf::Camera cam;
};

struct GLTFLight
{
	mat4 worldMatrix = GW::MATH::GIdentityMatrixF;

	tinygltf::Light light;
};

struct GLTFScene
{
	void ImportMaterials(const tinygltf::Model& model)
	{
		materials.reserve(model.materials.size());

		for (auto& mat : model.materials)
		{
			GLTFMaterial gMat;
			gMat.material = &mat;
			gMat.alphaCutoff = static_cast<float>(mat.alphaCutoff);
			gMat.alphaMode = mat.alphaMode == "MASK" ? 1 : (mat.alphaMode == "BLEND" ? 2 : 0);
			gMat.doubleSided = mat.doubleSided ? 1 : 0;
			gMat.emissiveFactor = mat.emissiveFactor.size() == 3 ? vec3{ (float)mat.emissiveFactor[0], (float)mat.emissiveFactor[1], (float)mat.emissiveFactor[2] } : vec3{ 0.f, 0.f, 0.f };
			gMat.emissiveTexture = mat.emissiveTexture.index;
			gMat.normalTexture = mat.normalTexture.index;
			gMat.normalTextureScale = static_cast<float>(mat.normalTexture.scale);
			gMat.occlusionTexture = mat.occlusionTexture.index;
			gMat.occlusionTextureStrength = static_cast<float>(mat.occlusionTexture.strength);

			//pbrMetallicRoughness
			auto& pbr = mat.pbrMetallicRoughness;
			gMat.baseColorFactor = { (float)pbr.baseColorFactor[0], (float)pbr.baseColorFactor[1], (float)pbr.baseColorFactor[2], (float)pbr.baseColorFactor[3] };
			gMat.baseColorTexture = pbr.baseColorTexture.index;
			gMat.metallicFactor = static_cast<float>(pbr.metallicFactor);
			gMat.metallicRoughnessTexture = pbr.metallicRoughnessTexture.index;
			gMat.roughnessFactor = static_cast<float>(pbr.roughnessFactor);

			//extensions
			// KHR_materials_specular
			if (mat.extensions.find("KHR_materials_specular") != mat.extensions.end())
			{
				const auto& ext = mat.extensions.find("KHR_materials_specular")->second;
				GetFloat(ext, "specularFactor", gMat.specular.specularFactor);
				GetTexId(ext, "specularTexture", gMat.specular.specularTexture);
				GetVec3(ext, "specularColorFactor", gMat.specular.specularColorFactor);
				GetTexId(ext, "specularColorTexture", gMat.specular.specularColorTexture);
			}

			// KHR_texture_transform
			if (pbr.baseColorTexture.extensions.find("KHR_texture_transform") != pbr.baseColorTexture.extensions.end())
			{
				const auto& ext = pbr.baseColorTexture.extensions.find("KHR_texture_transform")->second;
				auto& tt = gMat.textureTransform;
				GetVec2(ext, "offset", tt.offset);
				GetVec2(ext, "scale", tt.scale);
				GetFloat(ext, "rotation", tt.rotation);
				GetInt(ext, "texCoord", tt.texCoord);

				// Computing the transformation
				auto translation = mat3{ 1, 0, tt.offset.x, 0, 1, tt.offset.y, 0, 0, 1 };
				auto rotation = mat3{ cos(tt.rotation), sin(tt.rotation), 0, -sin(tt.rotation), cos(tt.rotation), 0, 0, 0, 1 };
				auto scale = mat3{ tt.scale.x, 0, 0, 0, tt.scale.y, 0, 0, 0, 1 };

				GW::MATH2D::GMatrix2D::Multiply3F(scale, rotation, tt.uvTransform);
				GW::MATH2D::GMatrix2D::Multiply3F(tt.uvTransform, translation, tt.uvTransform);
			}

			// KHR_materials_unlit
			if (mat.extensions.find("KHR_materials_unlit") != mat.extensions.end())
			{
				gMat.unlit.active = 1;
			}

			// KHR_materials_anisotropy
			if (mat.extensions.find("KHR_materials_anisotropy") != mat.extensions.end())
			{
				const auto& ext = mat.extensions.find("KHR_materials_anisotropy")->second;
				GetFloat(ext, "anisotropy", gMat.anisotropy.factor);
				GetVec3(ext, "anisotropyDirection", gMat.anisotropy.direction);
				GetTexId(ext, "anisotropyTexture", gMat.anisotropy.texture);
			}

			// KHR_materials_clearcoat
			if (mat.extensions.find("KHR_materials_clearcoat") != mat.extensions.end())
			{
				const auto& ext = mat.extensions.find("KHR_materials_clearcoat")->second;
				GetFloat(ext, "clearcoatFactor", gMat.clearcoat.factor);
				GetTexId(ext, "clearcoatTexture", gMat.clearcoat.texture);
				GetFloat(ext, "clearcoatRoughnessFactor", gMat.clearcoat.roughnessFactor);
				GetTexId(ext, "clearcoatRoughnessTexture", gMat.clearcoat.roughnessTexture);
				GetTexId(ext, "clearcoatNormalTexture", gMat.clearcoat.normalTexture);
			}

			// KHR_materials_sheen
			if (mat.extensions.find("KHR_materials_sheen") != mat.extensions.end())
			{
				const auto& ext = mat.extensions.find("KHR_materials_sheen")->second;
				GetVec3(ext, "sheenColorFactor", gMat.sheen.colorFactor);
				GetTexId(ext, "sheenColorTexture", gMat.sheen.colorTexture);
				GetFloat(ext, "sheenRoughnessFactor", gMat.sheen.roughnessFactor);
				GetTexId(ext, "sheenRoughnessTexture", gMat.sheen.roughnessTexture);
			}

			// KHR_materials_transmission
			if (mat.extensions.find("KHR_materials_transmission") != mat.extensions.end())
			{
				const auto& ext = mat.extensions.find("KHR_materials_transmission")->second;
				GetFloat(ext, "transmissionFactor", gMat.transmission.factor);
				GetTexId(ext, "transmissionTexture", gMat.transmission.texture);
			}

			// KHR_materials_ior
			if (mat.extensions.find("KHR_materials_ior") != mat.extensions.end())
			{
				const auto& ext = mat.extensions.find("KHR_materials_ior")->second;
				GetFloat(ext, "ior", gMat.ior.ior);
			}

			// KHR_materials_volume
			if (mat.extensions.find("KHR_materials_volume") != mat.extensions.end())
			{
				const auto& ext = mat.extensions.find("KHR_materials_volume")->second;
				GetFloat(ext, "thicknessFactor", gMat.volume.thicknessFactor);
				GetTexId(ext, "thicknessTexture", gMat.volume.thicknessTexture);
				GetFloat(ext, "attenuationDistance", gMat.volume.attenuationDistance);
				GetVec3(ext, "attenuationColor", gMat.volume.attenuationColor);
			}

			// KHR_materials_displacement
			if (mat.extensions.find("KHR_materials_displacement") != mat.extensions.end())
			{
				const auto& ext = mat.extensions.find("KHR_materials_displacement")->second;
				GetTexId(ext, "displacementGeometryTexture", gMat.displacement.displacementGeometryTexture);
				GetFloat(ext, "displacementGeometryFactor", gMat.displacement.displacementGeometryFactor);
				GetFloat(ext, "displacementGeometryOffset", gMat.displacement.displacementGeometryOffset);
			}

			// KHR_materials_emissive_strength
			if (mat.extensions.find("KHR_materials_emissive_strength") != mat.extensions.end())
			{
				const auto& ext = mat.extensions.find("KHR_materials_emissive_strength")->second;
				GetFloat(ext, "emissiveStrength", gMat.emissiveStrength.emissiveStrength);
			}

			materials.emplace_back(gMat);
		}

		// Make default
		if (materials.empty())
		{
			GLTFMaterial gMat;
			gMat.metallicFactor = 0;
			materials.emplace_back(gMat);
		}

	}

	void ImportDrawableNodes(const tinygltf::Model& model)
	{
		int defaultScene = model.defaultScene > -1 ? model.defaultScene : 0;
		const auto& scene = model.scenes[defaultScene];

		std::set<unsigned int> usedMeshes;

		for (auto nodeIdx : scene.nodes)
		{
			FindUsedMeshes(model, usedMeshes, nodeIdx);
		}

		unsigned int numberOfIndices = 0;
		unsigned int primitiveCount = 0;

		for (const auto& m : usedMeshes)
		{
			auto& mesh = model.meshes[m];
			std::vector<unsigned int> vPrim;

			for (const auto& primitive : mesh.primitives)
			{
				if (primitive.mode != 4) //triangle
				{
					continue;
				}

				const auto& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];

				if (primitive.indices > -1)
				{
					const auto& indexAccessor = model.accessors[primitive.indices];
					numberOfIndices += static_cast<unsigned int>(indexAccessor.count);
				}
				else
				{
					numberOfIndices += static_cast<unsigned int>(posAccessor.count);
				}
				vPrim.emplace_back(primitiveCount++);
			}

			meshToPrimMeshes[m] = std::move(vPrim); // mesh id
		}

		//reserve memory
		indices.reserve(numberOfIndices);

		//convert all mesh/primitives to a single prim per mesh
		for (const auto& m : usedMeshes)
		{
			auto& mesh = model.meshes[m];

			for (const auto& primitive : mesh.primitives)
			{
				ProcessMesh(model, primitive, mesh.name);
				primMeshes.back().mesh = &mesh;
				primMeshes.back().prim = &primitive;
			}
		}

		unsigned int numOfThreads = std::min((unsigned int)tangents.size(), std::thread::hardware_concurrency());
		ParallelBatches(tangents.size(), [&](unsigned long long i)
			{
				auto& t = tangents[i];
				float length2;
				GW::MATH::GVector::MagnitudeF(t, length2);
				if (length2 < 0.01f || std::abs(t.w) < 0.5f)
				{
					const auto& n = normals[i];
					const float sgn = n.z > 0.f ? 1.f : -1.f;
					const float a = -1.f / (sgn + n.z);
					const float b = n.x * n.y * a;
					t = { 1.f + sgn * n.x * n.x * a, sgn * b, -sgn * n.x,sgn };
				}
			}, numOfThreads);

		for (auto nodeIdx : scene.nodes)
		{
			ProcessNode(model, nodeIdx, GW::MATH::GIdentityMatrixF);
		}

		ComputeSceneDimensions();
		ComputeCamera();

		meshToPrimMeshes.clear();
		primitiveIndices32u.clear();
		primitiveIndices16u.clear();
		primitiveIndices8u.clear();

	}

	void ComputeSceneDimensions()
	{
		BoundingBox sceneBox;

		for (const auto& node : nodes)
		{
			const auto& mesh = primMeshes[node.primMesh];

			BoundingBox bBox(mesh.posMin, mesh.posMax);
			bBox = bBox.Transform(node.worldMatrix);
			sceneBox.Insert(bBox);
		}

		if (sceneBox.IsEmpty() || !sceneBox.isVolume())
		{
			std::cout << "glTF: Scene bounding box invalid, Setting to: [-1,-1,-1], [1,1,1]\n";
			sceneBox.Insert({ -1.0f, -1.0f, -1.0f });
			sceneBox.Insert({ 1.0f, 1.0f, 1.0f });
		}

		dimensions.min = sceneBox.Min();
		dimensions.max = sceneBox.Max();
		dimensions.size = sceneBox.Extents();
		dimensions.center = sceneBox.Center();
		dimensions.radius = sceneBox.Radius();

	}

	//scene data
	std::vector<GLTFMaterial> materials;
	std::vector<GLTFNode> nodes;
	std::vector<GLTFPrimMesh> primMeshes;
	std::vector<GLTFCamera> cameras;
	std::vector<GLTFLight> lights;

	//attributes
	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<vec4> tangents;
	std::vector<vec2> texCoords0;
	std::vector<vec4> colors;
	std::vector<unsigned int> indices;

	struct Dimensions
	{
		vec3 min = { std::numeric_limits<float>::max() , std::numeric_limits<float>::max() , std::numeric_limits<float>::max() };
		vec3 max = { std::numeric_limits<float>::min() , std::numeric_limits<float>::min() , std::numeric_limits<float>::min() };
		vec3 size = { 0, 0, 0 };
		vec3 center = { 0, 0, 0 };
		float radius = 0;
	} dimensions;

private:
	// Temporary data
	std::unordered_map<int, std::vector<uint32_t>> meshToPrimMeshes;
	std::vector<unsigned int> primitiveIndices32u;
	std::vector<unsigned short> primitiveIndices16u;
	std::vector<unsigned char> primitiveIndices8u;
	std::unordered_map<std::string, GLTFPrimMesh> cachePrimMesh;

	void FindUsedMeshes(const tinygltf::Model& model, std::set<uint32_t>& usedMeshes, int nodeIdx)
	{
		const auto& node = model.nodes[nodeIdx];

		if (node.mesh >= 0)
		{
			usedMeshes.insert(node.mesh);
		}

		for (const auto& c : node.children)
		{
			FindUsedMeshes(model, usedMeshes, c);
		}
	}

	void ProcessMesh(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const std::string& name)
	{
		if (primitive.mode != 4) return;

		GLTFPrimMesh resultMesh;
		resultMesh.name = name;
		resultMesh.materialIndex = std::max(0, primitive.material);
		resultMesh.vertexOffset = static_cast<unsigned int>(positions.size());
		resultMesh.firstIndex = static_cast<unsigned int>(indices.size());

		std::stringstream s;
		for (auto& a : primitive.attributes)
		{
			s << a.first << a.second;
		}

		std::string key = s.str();
		bool cached = false;

		auto it = cachePrimMesh.find(key);
		if (it != cachePrimMesh.end())
		{
			cached = true;
			GLTFPrimMesh cacheMesh = it->second;
			resultMesh.vertexCount = cacheMesh.vertexCount;
			resultMesh.vertexOffset = cacheMesh.vertexOffset;
		}

		//indices
		if (primitive.indices > -1)
		{
			const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
			const tinygltf::BufferView& bufferView = model.bufferViews[indexAccessor.bufferView];
			const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
			resultMesh.indexCount = static_cast<unsigned int>(indexAccessor.count);

			switch (indexAccessor.componentType)
			{
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
			{
				primitiveIndices32u.resize(indexAccessor.count);
				memcpy(primitiveIndices32u.data(), &buffer.data[indexAccessor.byteOffset + bufferView.byteOffset], indexAccessor.count * sizeof(unsigned int));
				indices.insert(indices.end(), primitiveIndices32u.begin(), primitiveIndices32u.end());
				break;
			}
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
			{
				primitiveIndices16u.resize(indexAccessor.count);
				memcpy(primitiveIndices16u.data(), &buffer.data[indexAccessor.byteOffset + bufferView.byteOffset], indexAccessor.count * sizeof(unsigned short));
				indices.insert(indices.end(), primitiveIndices16u.begin(), primitiveIndices16u.end());
				break;
			}
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
			{
				primitiveIndices8u.resize(indexAccessor.count);
				memcpy(primitiveIndices8u.data(), &buffer.data[indexAccessor.byteOffset + bufferView.byteOffset], indexAccessor.count * sizeof(unsigned char));
				indices.insert(indices.end(), primitiveIndices8u.begin(), primitiveIndices8u.end());
				break;
			}
			default:
				std::cout << "Index component type " << indexAccessor.componentType << " not supported!\n";
				return;
			}
		}
		else
		{
			// Primitive without indices, creating them
			const auto& accessor = model.accessors[primitive.attributes.find("POSITION")->second];
			for (auto i = 0; i < accessor.count; i++)
				indices.push_back(i);
			resultMesh.indexCount = static_cast<unsigned int>(accessor.count);
		}

		if (!cached) //need to add prim
		{
			//position
			{
				assert(primitive.attributes.find("POSITION") != primitive.attributes.end());
				const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("POSITION")->second];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

				auto position = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

				if (position)
				{
					for (size_t i = 0; i < accessor.count; i++)
					{
						positions.push_back(vec3{ position[i * 3 + 0], position[i * 3 + 1], position[i * 3 + 2] });
					}
				}

				resultMesh.vertexCount = static_cast<unsigned int>(accessor.count);

				if (!accessor.minValues.empty())
				{
					resultMesh.posMin = { (float)accessor.minValues[0], (float)accessor.minValues[1] , (float)accessor.minValues[2] };
				}
				else
				{
					resultMesh.posMin = { FLT_MAX, FLT_MAX, FLT_MAX };

					for (const auto& p : positions)
					{
						resultMesh.posMin.x = p.x < resultMesh.posMin.x ? p.x : FLT_MAX;
						resultMesh.posMin.y = p.y < resultMesh.posMin.y ? p.y : FLT_MAX;
						resultMesh.posMin.z = p.z < resultMesh.posMin.z ? p.z : FLT_MAX;
					}
				}

				if (!accessor.maxValues.empty())
				{
					resultMesh.posMax = { (float)accessor.maxValues[0], (float)accessor.maxValues[1] , (float)accessor.maxValues[2] };
				}
				else
				{
					resultMesh.posMin = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

					for (const auto& p : positions)
					{
						resultMesh.posMax.x = p.x < resultMesh.posMax.x ? p.x : -FLT_MAX;
						resultMesh.posMax.y = p.y < resultMesh.posMax.y ? p.y : -FLT_MAX;
						resultMesh.posMax.z = p.z < resultMesh.posMax.z ? p.z : -FLT_MAX;
					}
				}
			}

			//normal
			{
				const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

				auto normal = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

				if (normal)
				{
					for (size_t i = 0; i < accessor.count; i++)
					{
						GW::MATH2D::GVECTOR3F n;
						GVector2D::Normalize3F(GW::MATH2D::GVECTOR3F{ normal[i * 3 + 0], normal[i * 3 + 1], normal[i * 3 + 2] }, n);

						normals.emplace_back(n);
					}
				}
				else
					CreateNormals(resultMesh);
			}

			//texcoord_0
			{
				const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

				auto uv0 = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

				if (uv0)
				{
					for (size_t i = 0; i < accessor.count; i++)
					{
						texCoords0.push_back(vec2{ uv0[i * 2 + 0], uv0[i * 2 + 1] });
					}
				}
				else
					CreateTexCoords(resultMesh);
			}

			//tangent
			{
				const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("TANGENT")->second];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

				auto tangent = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

				if (tangent)
				{
					for (size_t i = 0; i < accessor.count; i++)
					{
						tangents.push_back(GW::MATH::GVECTORF{ tangent[i * 4 + 0], tangent[i * 4 + 1], tangent[i * 4 + 2], tangent[i * 4 + 3] });
					}
				}
				else
					CreateTangents(resultMesh);
			}

			//color
			{
				const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("TANGENT")->second];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

				auto color = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

				if (color)
				{
					for (size_t i = 0; i < accessor.count; i++)
					{
						colors.push_back(GW::MATH::GVECTORF{ color[i * 4 + 0], color[i * 4 + 1], color[i * 4 + 2], color[i * 4 + 3] });
					}
				}
				else
					colors.insert(colors.end(), resultMesh.vertexCount, GW::MATH::GVECTORF{ 1, 1, 1, 1 });
			}
		}

		cachePrimMesh[key] = resultMesh;

		primMeshes.emplace_back(resultMesh);
	}

	void ProcessNode(const tinygltf::Model& model, int& nodeIdx, const mat4& parentMatrix)
	{
		const auto& node = model.nodes[nodeIdx];

		mat4 matrix = GetLocalMatrix(node);
		mat4  world;
		{
			GMatrix::MultiplyMatrixF(parentMatrix, matrix, world);
		}

		if (node.mesh > -1)
		{
			const auto& meshes = meshToPrimMeshes[node.mesh]; //mesh can have many primitives

			for (const auto& mesh : meshes)
			{
				GLTFNode gNode;
				gNode.primMesh = mesh;
				gNode.worldMatrix = world;
				gNode.node = &node;

				nodes.emplace_back(gNode);
			}
		}
		else if (node.camera > -1)
		{
			GLTFCamera cam;
			cam.worldMatrix = world;
			cam.cam = model.cameras[model.nodes[nodeIdx].camera];

			if (node.extensions.find("NV_attributes_iray") != node.extensions.end())
			{
				auto& iRayExt = node.extensions.at("NV_attributes_iray");
				auto& attributes = iRayExt.Get("attributes");

				for (size_t i = 0; i < attributes.ArrayLen(); i++)
				{
					auto& attrib = attributes.Get((int)i);
					std::string attName = attrib.Get("name").Get<std::string>();
					auto& attValue = attrib.Get("value");

					if (attValue.IsArray())
					{
						auto vec = GetVector<float>(attValue);
						if (attName == "iview:position")
							cam.eye = { vec[0], vec[1], vec[2] };
						else if (attName == "iview:interest")
							cam.center = { vec[0], vec[1], vec[2] };
						else if (attName == "iview:up")
							cam.up = { vec[0], vec[1], vec[2] };
					}
				}
			}

			cameras.emplace_back(cam);
		}
		else if (node.extensions.find("KHR_lights_punctual") != node.extensions.end())
		{
			GLTFLight light;
			const auto& ext = node.extensions.find("KHR_lights_punctual")->second;
			auto        lightIdx = ext.Get("light").GetNumberAsInt();
			light.light = model.lights[lightIdx];
			light.worldMatrix = world;
			lights.emplace_back(light);
		}

		//recurse for childern
		for (auto child : node.children)
		{
			ProcessNode(model, child, world);
		}
	}

	void CreateNormals(GLTFPrimMesh& resultMesh)
	{
		std::vector <GW::MATH2D::GVECTOR3F> geonormal(resultMesh.vertexCount);

		for (size_t i = 0; i < resultMesh.indexCount; i += 3)
		{
			unsigned int ind0 = indices[resultMesh.firstIndex + i + 0];
			unsigned int ind1 = indices[resultMesh.firstIndex + i + 1];
			unsigned int ind2 = indices[resultMesh.firstIndex + i + 2];
			const auto& pos0 = positions[ind0 + resultMesh.vertexOffset];
			const auto& pos1 = positions[ind1 + resultMesh.vertexOffset];
			const auto& pos2 = positions[ind2 + resultMesh.vertexOffset];
			GW::MATH2D::GVECTOR3F v1;
			{
				GVector2D::Subtract3F(pos1, pos0, v1);
				GVector2D::Normalize3F(v1, v1);
			}
			GW::MATH2D::GVECTOR3F v2;
			{
				GVector2D::Subtract3F(pos2, pos0, v1);
				GVector2D::Normalize3F(v1, v1);
			}
			GW::MATH2D::GVECTOR3F n;
			{
				GVector2D::Cross3F(v1, v2, n);
			}

			GVector2D::Add3F(geonormal[ind0], n, geonormal[ind0]);
			GVector2D::Add3F(geonormal[ind1], n, geonormal[ind1]);
			GVector2D::Add3F(geonormal[ind2], n, geonormal[ind2]);
		}

		for (auto& n : geonormal)
		{
			GVector2D::Normalize3F(n, n);
		}

		normals.insert(normals.end(), geonormal.begin(), geonormal.end());
	}

	void CreateTexCoords(GLTFPrimMesh& resultMesh)
	{
		for (unsigned int i = 0; i < resultMesh.vertexCount; i++)
		{
			const auto& pos = positions[resultMesh.vertexOffset + i];
			float absX = fabs(pos.x);
			float absY = fabs(pos.y);
			float absZ = fabs(pos.z);

			bool isXPositive = pos.x > 0 ? true : false;
			bool isYPositive = pos.y > 0 ? true : false;
			bool isZPositive = pos.z > 0 ? true : false;

			float maxAxis{}, uc{}, vc{};

			//pos x
			if (isXPositive && absX >= absY && absX >= absZ)
			{
				maxAxis = absX;
				uc = -pos.z;
				vc = pos.y;
			}

			//neg x
			if (!isXPositive && absX >= absY && absX >= absZ)
			{
				maxAxis = absX;
				uc = pos.z;
				vc = pos.y;
			}

			//pos y
			if (isYPositive && absY >= absX && absY >= absZ)
			{
				maxAxis = absY;
				uc = pos.x;
				vc = -pos.z;
			}

			//neg y
			if (!isYPositive && absY >= absX && absY >= absZ)
			{
				maxAxis = absY;
				uc = pos.x;
				vc = pos.z;
			}

			//pos z
			if (isZPositive && absZ >= absX && absZ >= absY)
			{
				maxAxis = absZ;
				uc = pos.x;
				vc = pos.y;
			}

			//neg z
			if (!isZPositive && absZ >= absX && absZ >= absY)
			{
				maxAxis = absZ;
				uc = -pos.x;
				vc = pos.y;
			}

			// Convert range from -1 to 1 to 0 to 1
			float u = 0.5f * (uc / maxAxis + 1.0f);
			float v = 0.5f * (vc / maxAxis + 1.0f);

			texCoords0.emplace_back(vec2{ u, v });
		}
	}

	void CreateTangents(GLTFPrimMesh& resultMesh)
	{
		std::vector<GW::MATH2D::GVECTOR3F> tangent(resultMesh.vertexCount);
		std::vector<GW::MATH2D::GVECTOR3F> biTangent(resultMesh.vertexCount);

		for (size_t i = 0; i < resultMesh.indexCount; i += 3)
		{
			//local index
			unsigned int i0 = indices[resultMesh.firstIndex + i + 0];
			unsigned int i1 = indices[resultMesh.firstIndex + i + 1];
			unsigned int i2 = indices[resultMesh.firstIndex + i + 2];
			assert(i0 < resultMesh.vertexCount);
			assert(i1 < resultMesh.vertexCount);
			assert(i2 < resultMesh.vertexCount);

			//global index
			unsigned int gi0 = i0 + resultMesh.vertexOffset;
			unsigned int gi1 = i1 + resultMesh.vertexOffset;
			unsigned int gi2 = i2 + resultMesh.vertexOffset;

			const auto& p0 = positions[gi0];
			const auto& p1 = positions[gi1];
			const auto& p2 = positions[gi2];

			const auto& uv0 = texCoords0[gi0];
			const auto& uv1 = texCoords0[gi1];
			const auto& uv2 = texCoords0[gi2];

			GW::MATH2D::GVECTOR3F e1, e2;
			{
				GVector2D::Subtract3F(p1, p0, e1);
				GVector2D::Subtract3F(p2, p0, e2);
			}

			vec2 duvE1, duvE2;
			{
				GVector2D::Subtract2F(uv1, uv0, duvE1);
				GVector2D::Subtract2F(uv2, uv0, duvE2);
			}

			float r = 1.f;
			float a = duvE1.x * duvE2.y - duvE2.x * duvE1.y;

			if (fabs(a) > 0) //catch degenerated UVs
			{
				r = 1.f / a;
			}

			GW::MATH2D::GVECTOR3F t, b;
			{
				GW::MATH2D::GVECTOR3F v[3];

				//t
				GVector2D::Scale3F(e1, duvE2.y, v[0]);
				GVector2D::Scale3F(e2, duvE1.y, v[1]);
				GVector2D::Subtract3F(v[0], v[1], v[2]);
				GVector2D::Scale3F(v[2], r, t);

				//b
				GVector2D::Scale3F(e2, duvE1.x, v[0]);
				GVector2D::Scale3F(e1, duvE2.x, v[1]);
				GVector2D::Subtract3F(v[0], v[1], v[2]);
				GVector2D::Scale3F(v[2], r, b);
			}

			GVector2D::Add3F(tangent[i0], t, tangent[i0]);
			GVector2D::Add3F(tangent[i1], t, tangent[i1]);
			GVector2D::Add3F(tangent[i2], t, tangent[i2]);

			GVector2D::Add3F(biTangent[i0], b, biTangent[i0]);
			GVector2D::Add3F(biTangent[i1], b, biTangent[i1]);
			GVector2D::Add3F(biTangent[i2], b, biTangent[i2]);
		}

		for (unsigned int a = 0; a < resultMesh.vertexCount; a++)
		{
			const auto& t = tangent[a];
			const auto& b = biTangent[a];
			const auto& n = normals[resultMesh.vertexOffset + a];

			GW::MATH2D::GVECTOR3F oTangent;
			{
				GW::MATH2D::GVECTOR3F v;
				float d;
				GVector2D::Dot3F(n, t, d);
				GVector2D::Scale3F(n, d, v);
				GVector2D::Subtract3F(t, v, v);
				GVector2D::Normalize3F(v, oTangent);
			}

			if (oTangent.x == 0 && oTangent.y == 0 && oTangent.z == 0) //if tangent invalid
			{
				if (fabsf(n.x) > fabsf(n.y))
					GVector2D::Scale3F(GW::MATH2D::GVECTOR3F{ n.z, 0, -n.x }, 1 / sqrtf(n.x * n.x + n.z * n.z), oTangent);
				else
					GVector2D::Scale3F(GW::MATH2D::GVECTOR3F{ 0, -n.z, n.y }, 1 / sqrtf(n.y * n.y + n.z * n.z), oTangent);
			}

			//calculate handedness
			float handedness;
			{
				float f;
				GW::MATH2D::GVECTOR3F v;
				GVector2D::Cross3F(n, t, v);
				GVector2D::Dot3F(v, b, f);
				handedness = f < 0.f ? 1.f : -1.f;
			}

			tangents.emplace_back(GW::MATH::GVECTORF{ oTangent.x, oTangent.y, oTangent.z, handedness });
		}
	}

	void ComputeCamera()
	{
		for (auto& camera : cameras)
		{
			if (camera.eye.x == camera.center.x && camera.eye.y == camera.center.y && camera.eye.z == camera.center.z)
			{
				camera.eye = { camera.worldMatrix.row4.x, camera.worldMatrix.row4.y , camera.worldMatrix.row4.z };
				float distance;
				{
					vec3 v;
					GVector2D::Subtract3F(dimensions.center, camera.eye, v);
					GVector2D::Magnitude3F(v, distance);
				}

				mat3 rotMat;
				{
					rotMat.row1 = { camera.worldMatrix.row1.x, camera.worldMatrix.row1.y, camera.worldMatrix.row1.z };
					rotMat.row2 = { camera.worldMatrix.row2.x, camera.worldMatrix.row2.y, camera.worldMatrix.row2.z };
					rotMat.row3 = { camera.worldMatrix.row3.x, camera.worldMatrix.row3.y, camera.worldMatrix.row3.z };
				}

				camera.center = { 0, 0, -distance };
				{
					vec3 v;
					GW::MATH2D::GMatrix2D::MatrixXVector3F(rotMat, camera.center, v);
					GVector2D::Add3F(camera.eye, v, camera.center);
				}

				camera.up = { 0, 1, 0 };
			}
		}
	}

	mat4 GetLocalMatrix(const tinygltf::Node& node)
	{
		//translation
		GW::MATH::GVECTORF translation = { 0, 0, 0, 0 };
		if (node.translation.size() == 3)
		{
			translation = { (float)node.translation[0], (float)node.translation[1] , (float)node.translation[2] };
		}

		//rotation
		mat4 rotation = GW::MATH::GIdentityMatrixF;
		if (node.rotation.size() == 4)
		{
			GW::MATH::GQUATERNIONF quat = { (float)node.rotation[0], (float)node.rotation[1], (float)node.rotation[2], (float)node.rotation[3] };
			GMatrix::ConvertQuaternionF(quat, rotation);
			//GQuaternion::SetByMatrixF(rotation, rotation);
		}

		//scale
		GW::MATH::GVECTORF scale = { 1, 1, 1, 1 };
		if (node.scale.size() == 3)
		{
			scale = { (float)node.scale[0], (float)node.scale[1] , (float)node.scale[2] };
		}

		auto& matrix = GW::MATH::GIdentityMatrixF;

		mat4 translatedMatrix;
		mat4 scaledMatrix;
		mat4 rotatedMatrix;
		mat4 m;

		GMatrix::TranslateLocalF(GW::MATH::GIdentityMatrixF, translation, translatedMatrix);
		GMatrix::MultiplyMatrixF(translatedMatrix, rotation, rotatedMatrix);
		GMatrix::ScaleLocalF(GW::MATH::GIdentityMatrixF, scale, scaledMatrix);
		GMatrix::MultiplyMatrixF(rotatedMatrix, scaledMatrix, m);
		//GMatrix::MultiplyMatrixF(m, matrix, matrix);

		return matrix;
	}
};

struct PrimMeshInfo //retrieve primitive info during closest hit
{
	unsigned int idxOffset;
	unsigned int vertexOffset;
	int materialIndex;
};

struct SceneDescription
{
	unsigned long long vertexAddress; //vertex buffer address
	unsigned long long normalAddress; //normal buffer address
	unsigned long long uvAddress; //uv buffer address
	unsigned long long idxAddress; //index buffer address
	unsigned long long materialAddress; //material(GLTFShaderMaterial) buffer address
	unsigned long long primInfoAddress; //primitives(PrimMeshInfo) buffer address
};

struct GltfShaderMaterial
{
	// Core
	vec4  pbrBaseColorFactor;           // offset 0 - 16 bytes
	vec3  emissiveFactor;               // offset 16 - 12 bytes
	int pbrBaseColorTexture;          // offset 28 - 4 bytes
	int normalTexture;                // offset 32 - 4 bytes
	float normalTextureScale;           // offset 36 - 4 bytes
	int _pad0;                        // offset 40 - 4 bytes
	float pbrRoughnessFactor;           // offset 44 - 4 bytes
	float pbrMetallicFactor;            // offset 48 - 4 bytes
	int pbrMetallicRoughnessTexture;  // offset 52 - 4 bytes
	int emissiveTexture;              // offset 56 - 4 bytes
	int alphaMode;                    // offset 60 - 4 bytes
	float alphaCutoff;                  // offset 64 - 4 bytes

	// KHR_materials_transmission
	float transmissionFactor;   // offset 68 - 4 bytes
	int transmissionTexture;  // offset 72 - 4 bytes

	// KHR_materials_ior
	float ior;  // offset 76 - 4 bytes

	// KHR_materials_volume
	vec3 attenuationColor;     // offset 80 - 12 bytes
	float thicknessFactor;      // offset 92 - 4 bytes
	int thicknessTexture;     // offset 96 - 4 bytes
	float attenuationDistance;  // offset 100 - 4 bytes

	// KHR_materials_clearcoat
	float clearcoatFactor;            // offset 104 - 4 bytes
	float clearcoatRoughness;         // offset 108 - 4 bytes
	int clearcoatTexture;           // offset 112 - 4 bytes
	int clearcoatRoughnessTexture;  // offset 116 - 4 bytes
	int clearcoatNormalTexture;     // offset 120 - 4 bytes

	// KHR_materials_specular
	vec3 specularColorFactor;   // offset 124 - 12 bytes
	float specularFactor;        // offset 136 - 4 bytes
	int specularTexture;       // offset 140 - 4 bytes
	int specularColorTexture;  // offset 144 - 4 bytes

	// KHR_materials_unlit
	int unlit;  // offset 148 - 4 bytes

	// KHR_materials_iridescence
	float iridescenceFactor;            // offset 152 - 4 bytes
	int iridescenceTexture;           // offset 156 - 4 bytes
	float iridescenceThicknessMaximum;  // offset 160 - 4 bytes
	float iridescenceThicknessMinimum;  // offset 164 - 4 bytes
	int iridescenceThicknessTexture;  // offset 168 - 4 bytes
	float iridescenceIor;               // offset 172 - 4 bytes

	// KHR_materials_anisotropy
	float anisotropyStrength;  // offset 176 - 4 bytes
	int anisotropyTexture;   // offset 180 - 4 bytes
	float anisotropyRotation;  // offset 184 - 4 bytes

	// KHR_texture_transform
	mat3 uvTransform;  // offset 188 - 48 bytes (mat3 occupies 3 vec4, thus 3 * 16 bytes = 48 bytes)
	vec4 _pad3;        // offset 236 - 16 bytes (to cover the 36 bytes of the mat3 (C++))

	// KHR_materials_sheen
	int sheenColorTexture;      // offset 252 - 4 bytes
	float sheenRoughnessFactor;   // offset 256 - 4 bytes
	int sheenRoughnessTexture;  // offset 260 - 4 bytes
	vec3 sheenColorFactor;       // offset 264 - 12 bytes
	int _pad2;                  // offset 276 - 4 bytes (padding to align to 16 bytes)

	// Total size: 280 bytes
};
