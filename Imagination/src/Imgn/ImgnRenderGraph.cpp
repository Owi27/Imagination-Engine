#include "pch.hpp"
#include "ImgnRenderGraph.h"

namespace Imgn
{
	//std::string ImgnRenderGraph::MakeBufferKey(std::string_view pName, uint64_t pSize)
	//{
	//	return std::format("{}|size={}", pName, pSize);
	//}

	//std::string ImgnRenderGraph::MakeImageKey(std::string_view pName, uint32_t pWidth, uint32_t pHeight)
	//{
	//	return std::format("{}|width={}|height={}", pName, pWidth, pHeight);
	//}

	std::string ImgnRenderGraph::CreateRGBufferDesc(const std::string& pName, uint64_t pSize)
	{
		_bufferDesc[pName] = RGBufferDesc
		{
			.size = pSize
		};

		return pName;
	}

	std::string ImgnRenderGraph::CreateRGImageDesc(const std::string& pName, uint32_t pWidth, uint32_t pHeight, vk::Format pFormat)
	{
		_imageDesc[pName] = RGImageDesc
		{
			.width = pWidth,
			.height = pHeight,
			.format = pFormat
		};

		if (pName.contains("Depth"))
		{
			_images[pName] = _vk.CreateRenderImage(pWidth, pHeight, pFormat, vk::ImageAspectFlagBits::eDepth);
			
			return pName;
		}

		std::vector<uint8_t> newImageData(pWidth * pHeight * 4, 0); //all black image

		_images[pName] = _vk.CreateRenderImage(pWidth, pHeight, pFormat, vk::ImageAspectFlagBits::eColor);

		return pName;
	}

	//uint64_t ImgnRenderGraph::GetBufferSizeFromKey(std::string_view pKey)
	//{
	//	constexpr std::string_view token = "|size=";

	//	const uint32_t pos = pKey.rfind(token);

	//	if (pos == std::string_view::npos)
	//	{
	//		IMGN_FATAL("buffer key doesn't contain size: {}", pKey);
	//	}

	//	const std::string_view size = pKey.substr(pos + token.size());

	//	uint64_t bufferSize = 0;

	//	const auto [end, error] = std::from_chars(size.data(), size.data() + size.size(), bufferSize);

	//	return bufferSize;
	//}

	//uint32_t ImgnRenderGraph::GetImageWidthFromKey(std::string_view pKey)
	//{
	//	constexpr std::string_view token = "|width=";

	//	const uint32_t pos = pKey.find(token);

	//	if (pos == std::string_view::npos)
	//	{
	//		IMGN_FATAL("image key doesn't contain width: {}", pKey);
	//	}

	//	const size_t valueStart = pos + token.size();
	//	const size_t valueEnd = pKey.find('|', valueStart);

	//	const std::string_view widthText = pKey.substr(valueStart, valueEnd == std::string_view::npos ? std::string_view::npos : valueEnd - valueStart);

	//	uint32_t width = 0;

	//	const auto [end, error] = std::from_chars(widthText.data(), widthText.data() + widthText.size(), width);

	//	return width;
	//}

	//uint32_t ImgnRenderGraph::GetImageHeightFromKey(std::string_view pKey)
	//{
	//	constexpr std::string_view token = "|height=";

	//	const uint32_t position = pKey.rfind(token);

	//	if (position == std::string_view::npos)
	//	{
	//		IMGN_FATAL("image key doesn't contain height: {}", pKey);
	//	}

	//	const std::string_view heightText = pKey.substr(position + token.size());

	//	uint32_t height = 0;

	//	const auto [end, error] = std::from_chars(heightText.data(), heightText.data() + heightText.size(), height);

	//	return height;
	//}

	void ImgnRenderGraph::Compile()
	{
		std::vector<std::vector<size_t>> dependencies(_passes.size());  // What each pass depends on
		std::vector<std::vector<size_t>> dependents(_passes.size());    // What depends on each pass

		// Track which pass produces each resource (write-after-write dependencies)
		std::unordered_map<std::string_view, size_t> resourceWriters;

		// Dependency Discovery Through Resource Usage Analysis
		// Analyze each pass to determine data flow relationships
		for (int i = 0; auto& pass : _passes)
		{
			for (const auto& output : pass.imageOUT)
			{
				resourceWriters[output] = i; //set the idx of the pass that writes the output
			}

			for (const auto& output : pass.bufferOUT)
			{
				resourceWriters[output] = i; //set the idx of the pass that writes the output
			}

			i++;
		}

		for (int i = 0; auto& pass : _passes)
		{
			for (const auto& input : pass.imageIN)
			{
				auto it = resourceWriters.find(input);
				if (it != resourceWriters.end())
				{
					// Found the pass that produces this input - create dependency link
					dependencies[i].push_back(it->second);      // This pass depends on the producer
					dependents[it->second].push_back(i);        // Producer has this as dependent
				}
			}

			for (const auto& input : pass.bufferIN)
			{
				auto it = resourceWriters.find(input);
				if (it != resourceWriters.end())
				{
					// Found the pass that produces this input - create dependency link
					dependencies[i].push_back(it->second);      // This pass depends on the producer
					dependents[it->second].push_back(i);        // Producer has this as dependent
				}
			}

			i++;
		}

		// Topological Sort for Optimal Execution Order
		// Use depth-first search to compute valid execution sequence while detecting cycles
		std::vector<bool> visited(_passes.size(), false);       // Track completed nodes
		std::vector<bool> inStack(_passes.size(), false);       // Track current recursion path

		std::function<void(size_t)> visit = [&](size_t node)
			{
				if (inStack[node])
				{
					// Cycle detection - circular dependency found
					IMGN_FATAL("Cycle detected in RenderGraph")
						throw std::runtime_error("Cycle detected in rendergraph");
				}

				if (visited[node])
				{
					return;  // Already processed this node and its dependencies
				}

				inStack[node] = true;   // Mark as currently being processed

				// Recursively process all dependent passes first (post-order traversal)
				for (auto dependency : dependencies[node])
				{
					visit(dependency);
				}

				inStack[node] = false;  // Remove from current path
				visited[node] = true;   // Mark as completely processed

				_executionOrder.push_back(node);  // Add to execution sequence
			};

		// Process all unvisited nodes to handle disconnected graph components
		for (size_t i = 0; i < _passes.size(); i++)
		{
			if (!visited[i])
			{
				visit(i);
			}
		}

		//auto& device = ; TODO
		// Automatic Synchronization Object Creation
		   // Generate semaphores for all dependencies identified during analysis
		//for (size_t i = 0; i < _passes.size(); i++)
		//{
		//	for (auto dep : dependencies[i])
		//	{
		//		// Create a GPU semaphore for this dependency relationship
		//		// The dependent pass will wait on this semaphore before executing
		//		_semaphores.emplace_back(Unique<vk::raii::Semaphore>(std::move(_vk.CreateVkSemaphore())));
		//		_semaphoreSignalWaitPairs.emplace_back(dep, i);    // (producer, consumer) pair
		//	}
		//}

		// Physical Resource Allocation and Creation
		// Transform resource descriptions into actual GPU objects
		for (auto& pass : _passes)
		{
			for (auto& imageOUT : pass.imageOUT)
			{
				if (imageOUT.contains("Depth"))
				{
					_images[imageOUT] = _vk.CreateRenderImage(_imageDesc[imageOUT].width, _imageDesc[imageOUT].height, _imageDesc[imageOUT].format, vk::ImageAspectFlagBits::eDepth);
					continue;
				}

				std::vector<uint8_t> newImageData(_imageDesc[imageOUT].width* _imageDesc[imageOUT].height * 4, 0); //all black image

				_images[imageOUT] = _vk.CreateRenderImage(_imageDesc[imageOUT].width, _imageDesc[imageOUT].height, _imageDesc[imageOUT].format, vk::ImageAspectFlagBits::eColor);
			}
		}

		for (auto& pass : _passes)
		{
			for (auto& bufferOUT : pass.bufferOUT)
			{
				if (bufferOUT.contains("UB")) _buffers[bufferOUT] = _vk.CreateRenderBuffer(nullptr, _bufferDesc[bufferOUT].size, vk::BufferUsageFlagBits::eUniformBuffer);
				if (bufferOUT.contains("SB")) _buffers[bufferOUT] = _vk.CreateRenderBuffer(nullptr, _bufferDesc[bufferOUT].size, vk::BufferUsageFlagBits::eStorageBuffer);

			}
		}
	}

	void ImgnRenderGraph::Execute(vk::raii::CommandBuffer& pCommandBuffer)
	{
		for (auto passIdx : _executionOrder)
		{
			const auto& pass = _passes[passIdx];

			for (auto& input : pass.bufferIN)
			{
				RGBuffer& resource = _buffers[input];

				vk::BufferMemoryBarrier2 barrier
				{
					.srcStageMask = resource.currentStage,
					.srcAccessMask = resource.currentAccess,
					.dstStageMask = vk::PipelineStageFlagBits2::eAllGraphics,
					.dstAccessMask = vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eUniformRead,
					.buffer = *resource.buffer.buffer,
					.offset = 0,
					.size = VK_WHOLE_SIZE
				};

				pCommandBuffer.pipelineBarrier2(vk::DependencyInfo{ .bufferMemoryBarrierCount = 1, .pBufferMemoryBarriers = &barrier });

				resource.currentStage = vk::PipelineStageFlagBits2::eAllGraphics;
				resource.currentAccess = vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eUniformRead;
			}


			for (auto& input : pass.imageIN)
			{
				RGImage& resource = _images[input];

				if (resource.currentLayout != vk::ImageLayout::eShaderReadOnlyOptimal)
				{
					_vk.TransitionImageLayout(pCommandBuffer, resource.currentLayout, vk::ImageLayout::eShaderReadOnlyOptimal, *resource.image.image, resource.aspect);
					resource.currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				}
			}

			for (auto& output : pass.imageOUT)
			{
				RGImage& resource = _images[output];
				vk::ImageLayout target = (resource.aspect & vk::ImageAspectFlagBits::eColor) ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eDepthStencilAttachmentOptimal;

				if (resource.currentLayout != target)
				{
					_vk.TransitionImageLayout(pCommandBuffer, resource.currentLayout, target, *resource.image.image, resource.aspect);
					resource.currentLayout = target;
				}
			}

			if (!pass.Execute)
			{
				throw std::runtime_error(
					std::format("Render pass '{}' has no Execute callback", pass.name));
			}

			pass.Execute(pCommandBuffer);

			for (auto& output : pass.bufferOUT)
			{
				RGBuffer& resource = _buffers[output];

				vk::BufferMemoryBarrier2 barrier
				{
					.srcStageMask = resource.currentStage,
					.srcAccessMask = resource.currentAccess,
					.dstStageMask = vk::PipelineStageFlagBits2::eAllGraphics,
					.dstAccessMask = vk::AccessFlagBits2::eShaderWrite,
					.buffer = *resource.buffer.buffer,
					.offset = 0,
					.size = VK_WHOLE_SIZE
				};

				pCommandBuffer.pipelineBarrier2(vk::DependencyInfo{ .bufferMemoryBarrierCount = 1, .pBufferMemoryBarriers = &barrier });

				resource.currentStage = vk::PipelineStageFlagBits2::eComputeShader;
				resource.currentAccess = vk::AccessFlagBits2::eShaderWrite;
			}
		}

		for (auto& [name, resource] : _images)
		{
			if (resource.aspect & vk::ImageAspectFlagBits::eDepth)
			{
				if (resource.currentLayout != vk::ImageLayout::eDepthStencilAttachmentOptimal)
				{
					_vk.TransitionImageLayout(pCommandBuffer, resource.currentLayout, vk::ImageLayout::eDepthStencilAttachmentOptimal, *resource.image.image, resource.aspect);
					resource.currentLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
				}

				continue;
			}

			if (resource.currentLayout != vk::ImageLayout::eShaderReadOnlyOptimal)
			{
				_vk.TransitionImageLayout(pCommandBuffer, resource.currentLayout, vk::ImageLayout::eShaderReadOnlyOptimal, *resource.image.image, resource.aspect);
				resource.currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			}
		}
	}

	void ImgnRenderGraph::AddPass(RenderPass& pPass)
	{
		_passes.push_back(pPass);
	}
}