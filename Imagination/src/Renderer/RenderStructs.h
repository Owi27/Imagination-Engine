#pragma once

struct Vertex
{
	vec3 pos;
	vec3 nrm;
	vec2 uv0;
	vec4 tan;
	vec3 clr = { 1.0f, 1.0f, 1.0f };
	uvec4 joints;
	vec4 weights;

	//vk
	static vk::VertexInputBindingDescription GetBindingDescription() { return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex }; }
	static std::array<vk::VertexInputAttributeDescription, 5> GetAttributeDescriptions()
	{
		return
		{
			vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
			vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, nrm)),
			vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv0)),
			vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, tan)),
			vk::VertexInputAttributeDescription(4, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, clr)),
		};
	}
};

namespace Imgn
{
	struct Material
	{
		std::array<float, 4> baseColorFactor;
		std::array<float, 4> emissiveFactor;

		// x = base color
		// y = metallic/roughness
		// z = emissive
		// w = normal
		std::array<int32_t, 4> textureIndices0;

		// x = occlusion
		// y = alpha mode
		// z = double sided
		// w = unused
		std::array<int32_t, 4> textureIndices1;

		// x = metallic
		// y = roughness
		// z = alpha cutoff
		// w = normal scale
		std::array<float, 4> materialFactors;

		// x = occlusion strength
		// yzw = unused
		std::array<float, 4> extraFactors;
	};
}

enum class RendererBackend
{
	Vulkan, D3D12, Metal
};

struct RendererCreateInfo
{
	RendererBackend backend = RendererBackend::Vulkan;

	void* windowHandle = nullptr, * displayHandle = nullptr;

	uint32_t width = 1280, height = 720;

	bool enableValidation = true;
};
