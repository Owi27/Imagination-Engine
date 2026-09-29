#include "pch.hpp"
#include "Imgn/UI/UILayout.h"

#include "Imgn/UI/UICanvas.h"

namespace Imgn
{
	void UILayoutSystem::CalculateLayout(UICanvas* pCanvas)
	{
		if (!pCanvas) return;
		if (pCanvas->GetMode() == eUICanvasMode::WorldSpace) return;

		const UIRect screen = pCanvas->ContentRect();
		//the canvas root fills the screen. children are authored in reference pixels and scaled.
		pCanvas->GetTransform().anchorMin = { 0.f, 0.f };
		pCanvas->GetTransform().anchorMax = { 0.f, 0.f };
		pCanvas->GetTransform().pivot = { 0.f, 0.f };
		pCanvas->GetTransform().position = { 0.f, 0.f };
		pCanvas->GetTransform().size = { screen.width, screen.height };

		pCanvas->LayoutAsRoot(screen, pCanvas->GetScale());
	}

	void UIHorizontalLayout::ArrangeChildren()
	{
		const float scale = GetCanvasScale() > 0.f ? GetCanvasScale() : 1.f;
		float cursor = _padding;

		for (const unique<UIElement>& child : GetChildren())
		{
			if (!child) continue;

			UITransform& transform = child->GetTransform();
			transform.anchorMin = { 0.f, 0.f };
			transform.anchorMax = { 0.f, 0.f };
			transform.pivot = { 0.f, 0.f };
			transform.position = { cursor, _padding };

			float width = transform.size[0];
			if (width <= 0.f) width = 100.f;
			transform.size[0] = width;
			cursor += width + _spacing;
			(void)scale;
		}
	}

	void UIVerticalLayout::ArrangeChildren()
	{
		float cursor = _padding;

		for (const unique<UIElement>& child : GetChildren())
		{
			if (!child) continue;

			UITransform& transform = child->GetTransform();
			transform.anchorMin = { 0.f, 0.f };
			transform.anchorMax = { 0.f, 0.f };
			transform.pivot = { 0.f, 0.f };
			transform.position = { _padding, cursor };

			float height = transform.size[1];
			if (height <= 0.f) height = 32.f;
			transform.size[1] = height;
			cursor += height + _spacing;
		}
	}

	void UIGridLayout::ArrangeChildren()
	{
		const uint32_t columns = _columns < 1 ? 1 : _columns;
		uint32_t index = 0;

		for (const unique<UIElement>& child : GetChildren())
		{
			if (!child) continue;

			const uint32_t column = index % columns;
			const uint32_t row = index / columns;
			const float x = _padding + column * (_cellSize[0] + _spacing[0]);
			const float y = _padding + row * (_cellSize[1] + _spacing[1]);

			UITransform& transform = child->GetTransform();
			transform.anchorMin = { 0.f, 0.f };
			transform.anchorMax = { 0.f, 0.f };
			transform.pivot = { 0.f, 0.f };
			transform.position = { x, y };
			transform.size = _cellSize;
			index++;
		}
	}

	void UIOverlayLayout::ArrangeChildren()
	{
		for (const unique<UIElement>& child : GetChildren())
		{
			if (!child) continue;

			UITransform& transform = child->GetTransform();
			transform.anchorMin = { 0.f, 0.f };
			transform.anchorMax = { 1.f, 1.f };
			transform.pivot = { 0.5f, 0.5f };
			transform.position = { 0.f, 0.f };
			transform.size = { 0.f, 0.f };
		}
	}
}
