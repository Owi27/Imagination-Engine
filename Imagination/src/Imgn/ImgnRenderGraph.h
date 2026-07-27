#pragma once
#include "Renderer/Vulkan.hpp"

namespace Imgn
{
	struct IMGN_API RenderPass
	{
		std::string name;
		std::vector<std::string_view> imageIN;
		std::vector<std::string_view> imageOUT;
		std::vector<std::string_view> bufferIN;
		std::vector<std::string_view> bufferOUT;

		std::function<void(vk::raii::CommandBuffer&)> Execute;
	};

	class ImgnRenderGraph
	{
		Vulkan& _vk;

		std::vector<uint64_t> _executionOrder;
		std::vector<RenderPass> _passes;
		std::unordered_map<std::string_view, Image> _images;
		std::unordered_map<std::string_view, Buffer> _buffers;

		std::vector<unique<vk::raii::Semaphore>> _semaphores;
		std::vector<std::pair<uint64_t, uint64_t>> _semaphoreSignalWaitPairs;

		uint64_t GetBufferSizeFromKey(std::string_view pKey);
		uint32_t GetImageWidthFromKey(std::string_view pKey);
		uint32_t GetImageHeightFromKey(std::string_view pKey);

	public:
		uint32_t width = 1, height = 1;

		ImgnRenderGraph(Vulkan& pVk) : _vk(pVk)
		{

		}

		~ImgnRenderGraph()
		{

		}

		std::string MakeBufferKey(std::string_view pName, uint64_t pSize);
		std::string MakeImageKey(std::string_view pName, uint32_t pWidth, uint32_t pHeight);

		Image& GetImage(std::string_view pName) { return _images[pName]; }
		Buffer& GetBuffer(std::string_view pName) { return _buffers[pName]; }

		void Compile();

		void AddPass(RenderPass& pPass);
	};
}