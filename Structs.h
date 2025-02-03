#pragma once

struct Buffer
{
	VkBuffer buffer;
	VkDeviceMemory memory;
};

struct Image
{
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory memory;
};

struct Texture
{
	Image texImage;
	VkImageView texImageView;
	VkSampler texSampler;
};

struct FrameBufferTexture
{
	Texture texture;
	VkFormat format;
};

struct FrameBuffer
{
	VkFramebuffer frameBuffer;
	FrameBufferTexture position, normal, albedo, depth;
	VkRenderPass renderPass;
};

struct FrameBufferT
{
	VkRenderPass renderPass;
	VkFramebuffer frameBuffer;
	VkPipelineBindPoint bindPoint;
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	std::vector<VkShaderModule> shaderModules;
	//std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkAttachmentReference> attachmentReferences;
};

struct GeometryData
{
	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<vec2> texCoords;
	std::vector<vec4> tangents;
	std::vector<unsigned int> indices;
};
struct Light
{
	vec3 pos;
	float pad;
	vec3 col;
	float radius;
};

struct UniformBufferOffscreen
{
	mat4 world, view, proj;
	float deltaTime;
	vec3 pad;
};

struct UniformBufferFinal
{
	Light lights[10];
	vec4 view;
	mat4 viewProj;
};

struct Vertex
{
	vec3* pos, nrm;
	vec2* texCoord;
	vec4* tangent;
};

struct PCR
{
	mat4 model;
};

struct PrimData
{
	unsigned int firstIndex = 0;
	unsigned int indexCount = 0;
	unsigned int vertexOffset = 0;
	unsigned int vertexCount = 0;
	int materialIndex = 0;
};

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

struct Dimensions
{
	vec3 min = { std::numeric_limits<float>::max() , std::numeric_limits<float>::max() , std::numeric_limits<float>::max() };
	vec3 max = { std::numeric_limits<float>::min() , std::numeric_limits<float>::min() , std::numeric_limits<float>::min() };
	vec3 size = { 0, 0, 0 };
	vec3 center = { 0, 0, 0 };
	float radius = 0;
};

struct DrawInfo
{
	unsigned int idxCount, firstIdx, vertexOffset;
	mat4 nodeWorld;
};