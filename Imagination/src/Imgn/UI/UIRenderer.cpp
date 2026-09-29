#include "pch.hpp"
#include "Imgn/UI/UIRenderer.h"

#include "Imgn/ImgnRenderer.h"
#include "Imgn/ImgnRenderContext.h"

#include <array>
#include <cmath>

namespace Imgn
{
	bool UIRenderer::Init(ImgnRenderer* pRenderer)
	{
		if (!pRenderer) return false;
		_renderer = pRenderer;

		const uint64_t bytes = sizeof(UIQuadGPU) * MaxUIQuads;
		_quadBuffers[0] = _renderer->CreateMappedStorageBuffer(bytes);
		_quadBuffers[1] = _renderer->CreateMappedStorageBuffer(bytes);
		_quads.reserve(256);
		return true;
	}

	void UIRenderer::Render(RenderContext& pContext, uint32_t pWidth, uint32_t pHeight)
	{
		if (!_renderer || pWidth == 0 || pHeight == 0) return;
		UploadAndDraw(pContext, pWidth, pHeight);
	}

	void UIRenderer::UploadAndDraw(RenderContext& pContext, uint32_t pWidth, uint32_t pHeight)
	{
		std::vector<UIQuadGPU> gpu;
		gpu.reserve(_quads.size() + 1);

		UIQuadGPU scene;
		scene.rect[0] = 0.f;
		scene.rect[1] = 0.f;
		scene.rect[2] = static_cast<float>(pWidth);
		scene.rect[3] = static_cast<float>(pHeight);
		scene.uv[0] = 0.f; scene.uv[1] = 0.f; scene.uv[2] = 1.f; scene.uv[3] = 1.f;
		scene.color[0] = 1.f; scene.color[1] = 1.f; scene.color[2] = 1.f; scene.color[3] = 1.f;
		scene.extra[0] = -1.f;
		scene.extra[1] = UISceneBlitFlag;
		scene.extra[2] = 0.f;
		scene.extra[3] = 0.f;
		gpu.push_back(scene);

		struct Batch
		{
			uint32_t first = 0;
			uint32_t count = 0;
			int x = 0, y = 0, width = 0, height = 0;
		};
		std::vector<Batch> batches;
		Batch current;
		current.first = 0;
		current.count = 1;
		current.x = 0;
		current.y = 0;
		current.width = static_cast<int>(pWidth);
		current.height = static_cast<int>(pHeight);

		auto Flush = [&]()
		{
			if (current.count == 0) return;
			batches.push_back(current);
		};

		for (const UIDrawQuad& quad : _quads)
		{
			if (gpu.size() >= MaxUIQuads)
			{
				if (!_warned)
				{
					IMGN_CORE_WARN("UI quad list hit MaxUIQuads ({}). Extra widgets were dropped.", MaxUIQuads);
					_warned = true;
				}
				break;
			}

			int x = static_cast<int>(std::floor(quad.clipMinX));
			int y = static_cast<int>(std::floor(quad.clipMinY));
			int x2 = static_cast<int>(std::ceil(quad.clipMaxX));
			int y2 = static_cast<int>(std::ceil(quad.clipMaxY));
			if (x < 0) x = 0;
			if (y < 0) y = 0;
			if (x2 > static_cast<int>(pWidth)) x2 = static_cast<int>(pWidth);
			if (y2 > static_cast<int>(pHeight)) y2 = static_cast<int>(pHeight);
			const int width = x2 - x;
			const int height = y2 - y;
			if (width <= 0 || height <= 0) continue;

			if (current.count > 0 && (x != current.x || y != current.y || width != current.width || height != current.height))
			{
				Flush();
				current = {};
				current.first = static_cast<uint32_t>(gpu.size());
				current.x = x;
				current.y = y;
				current.width = width;
				current.height = height;
			}

			gpu.push_back(quad.gpu);
			current.count++;
		}
		Flush();

		const uint32_t frame = _renderer->GetFrameInFlightIndex() % 2;
		const uint32_t buffer = _quadBuffers[frame];
		_renderer->MapBufferData(buffer, gpu.data(), gpu.size() * sizeof(UIQuadGPU));

		std::array colorAttachments = { pContext.CreateRenderingAttachmentInfo("UIComposite") };
		pContext.BeginRendering(pWidth, pHeight, colorAttachments, nullptr);
		pContext.BindPipeline(vk::PipelineBindPoint::eGraphics, *_renderer->GetPipelines().uiPipeline);
		pContext.BindDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), 1, *_renderer->GetTextureDescriptorSet());
		pContext.SetViewport(pWidth, pHeight);

		vk::DescriptorBufferInfo quads = pContext.CreateDescriptorBufferInfo(buffer, sizeof(UIQuadGPU) * MaxUIQuads);
		vk::DescriptorImageInfo sceneImage = pContext.CreateDescriptorImageInfo("TAAResolved");
		vk::DescriptorImageInfo sampler = pContext.CreateSamplerInfo(_renderer->GetTextureSampler());
		std::array writes =
		{
			pContext.CreateWriteDescriptorSet(1, vk::DescriptorType::eStorageBuffer, quads),
			pContext.CreateWriteDescriptorSet(2, vk::DescriptorType::eSampledImage, sceneImage),
			pContext.CreateWriteDescriptorSet(4, vk::DescriptorType::eSampler, sampler)
		};
		pContext.PushDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), writes);

		for (const Batch& batch : batches)
		{
			if (batch.count == 0) continue;
			pContext.SetScissorRect(batch.x, batch.y, static_cast<uint32_t>(batch.width), static_cast<uint32_t>(batch.height));

			UIPC constants;
			constants.targetWidth = static_cast<float>(pWidth);
			constants.targetHeight = static_cast<float>(pHeight);
			constants.firstQuad = batch.first;
			constants.pad = 0;
			pContext.PushConstants<UIPC>(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, constants);
			pContext.DrawInstanced(6, batch.count);
		}

		pContext.EndRendering();
	}
}
