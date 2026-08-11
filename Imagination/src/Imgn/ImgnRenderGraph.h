#pragma once
#include "Renderer/Vulkan.hpp"

namespace Imgn
{
	struct IMGN_API RenderPass
	{
		std::string name;
		vk::PipelineBindPoint bindPoint;

		std::vector<std::string> imageIN;
		std::vector<std::string> imageOUT;
		std::vector<std::string> bufferIN;
		std::vector<std::string> bufferOUT;

		std::function<void(vk::raii::CommandBuffer&/*, uint32_t*/)> Execute;
	};

	class ImgnRenderGraph
	{
		struct RGImageDesc
		{
			uint32_t width, height;
			vk::Format format;
		};

		struct RGBufferDesc
		{
			uint64_t size;
		};

		struct RenderGraphFrameResources
		{
			std::unordered_map<std::string, RGImage> images;
			std::unordered_map<std::string, RGBuffer> buffers;
		};
		
		Vulkan& _vk;

		std::vector<uint64_t> _executionOrder;
		std::vector<RenderPass> _passes;
		std::unordered_map<std::string, RGImage> _images;
		std::unordered_map<std::string, RGBuffer> _buffers;
		std::unordered_map<std::string, RGImageDesc> _imageDesc;
		std::unordered_map<std::string, RGBufferDesc> _bufferDesc;
		std::array<RenderGraphFrameResources, 2> _frameResources;

		std::vector<unique<vk::raii::Semaphore>> _semaphores;
		std::vector<std::pair<uint64_t, uint64_t>> _semaphoreSignalWaitPairs;

		/*uint64_t GetBufferSizeFromKey(std::string_view pKey);
		uint32_t GetImageWidthFromKey(std::string_view pKey);
		uint32_t GetImageHeightFromKey(std::string_view pKey);*/

	public:
		ImgnRenderGraph(Vulkan& pVk) : _vk(pVk)
		{

		}

		~ImgnRenderGraph()
		{

		}

		//std::string MakeBufferKey(std::string_view pName, uint64_t pSize);
		//std::string MakeImageKey(std::string_view pName, uint32_t pWidth, uint32_t pHeight);

		std::string CreateRGBufferDesc(const std::string& pName, uint64_t pSize);
		std::string CreateRGImageDesc(const std::string& pName, uint32_t pWidth, uint32_t pHeight, vk::Format pFormat);

		RGImage& GetImage(std::string_view pName)
		{
			auto it = _images.find(std::string(pName));

			if (it == _images.end())
				throw std::runtime_error(
					std::format("RenderGraph image not found: {}", pName));

			return it->second;
		}

		RGBuffer& GetBuffer(std::string_view pName)
		{
			auto it = _buffers.find(std::string(pName));

			if (it == _buffers.end())
				throw std::runtime_error(
					std::format("RenderGraph buffer not found: {}", pName));

			return it->second;
		}
		void Compile();
		void Execute(vk::raii::CommandBuffer& pCommandBuffer);

		void AddPass(RenderPass& pPass);
	};
}