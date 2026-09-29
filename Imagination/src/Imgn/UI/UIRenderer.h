#pragma once
#include "Imgn/UI/UIElement.h"

namespace Imgn
{
	class ImgnRenderer;
	class RenderContext;

	class IMGN_API UIRenderer
	{
		ImgnRenderer* _renderer = nullptr;
		uint32_t _quadBuffers[2] = { 0, 0 };
		std::vector<UIDrawQuad> _quads;
		bool _warned = false;

		void UploadAndDraw(RenderContext& pContext, uint32_t pWidth, uint32_t pHeight);

	public:
		UIRenderer() = default;
		~UIRenderer() = default;

		UIRenderer(const UIRenderer&) = delete;
		UIRenderer& operator=(const UIRenderer&) = delete;

		bool Init(ImgnRenderer* pRenderer);
		void Clear() { _quads.clear(); }
		std::vector<UIDrawQuad>& Quads() { return _quads; }
		void AddQuad(const UIDrawQuad& pQuad) { _quads.push_back(pQuad); }
		void Render(RenderContext& pContext, uint32_t pWidth, uint32_t pHeight);
	};
}
