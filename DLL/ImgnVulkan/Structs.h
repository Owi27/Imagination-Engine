#pragma once
#include "ImgnVulkanAPI.h"

namespace ImgnVulkan
{
	struct IMGN_VULKAN_API Buffer
	{
		vk::raii::Buffer buffer = nullptr;
		vk::raii::DeviceMemory memory = nullptr;
	};

	struct IMGN_VULKAN_API Image
	{
		vk::raii::Image image = nullptr;
		vk::raii::ImageView view = nullptr;
		vk::raii::DeviceMemory memory = nullptr;
	};

	struct IMGN_VULKAN_API Pipelines
	{
		vk::raii::Pipeline gBufferPipeline = nullptr, lightingPipeline = nullptr, generalPipeline = nullptr;
		vk::raii::PipelineLayout pipelineLayout = nullptr;
	};

#pragma pack(push, 1)
	struct IMGN_VULKAN_API Vertex
	{
		::std::array<float, 3> pos;
		::std::array<float, 3> nrm;
		::std::array<float, 2> uv0;
		::std::array<float, 4> tan;
		::std::array<float, 3> col;

		static vk::VertexInputBindingDescription GetBindingDescription() { return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex }; }
		static ::std::array<vk::VertexInputAttributeDescription, 5> GetAttributeDescriptions()
		{
			return
			{
				vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
				vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, nrm)),
				vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv0)),
				vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, tan)),
				vk::VertexInputAttributeDescription(4, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, col)),
			};
		}

		bool operator==(const Vertex& other) const
		{
			return pos == other.pos && nrm == other.nrm && uv0 == other.uv0 && tan == other.tan && col == other.col;
		}
	};
#pragma pack(pop)
}

namespace std
{
	template<> struct hash<ImgnVulkan::Vertex>
	{
		uint64_t operator()(const ImgnVulkan::Vertex& vertex) const noexcept
		{
			uint64_t seed = 0;
			std::hash<float> hasher;

			seed ^= hasher(vertex.pos[0]) + hasher(vertex.pos[1]) + hasher(vertex.pos[2]);
			seed ^= hasher(vertex.nrm[0]) + hasher(vertex.nrm[1]) + hasher(vertex.nrm[2]);
			seed ^= hasher(vertex.uv0[0]) + hasher(vertex.uv0[1]);
			seed ^= hasher(vertex.tan[0]) + hasher(vertex.tan[1]) + hasher(vertex.tan[2]);

			return seed;
		}
	};
}