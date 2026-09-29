#include "pch.hpp"
#include "Imgn/UI/UIElement.h"

#include <algorithm>
#include <cmath>

namespace Imgn
{
	namespace
	{
		float ClampSize(float pSize, float pMin, float pMax)
		{
			if (pSize < pMin) pSize = pMin;
			if (pMax > 0.f && pSize > pMax) pSize = pMax;
			return pSize;
		}

		UIRect IntersectClip(float pMinX, float pMinY, float pMaxX, float pMaxY, const UIRect& pRect)
		{
			UIRect clip;
			clip.x = std::max(pMinX, pRect.x);
			clip.y = std::max(pMinY, pRect.y);
			clip.width = std::min(pMaxX, pRect.Right()) - clip.x;
			clip.height = std::min(pMaxY, pRect.Bottom()) - clip.y;
			if (clip.width < 0.f) clip.width = 0.f;
			if (clip.height < 0.f) clip.height = 0.f;
			return clip;
		}
	}

	void UIElement::Dream(float pDeltaTime)
	{
		if (!_enabled) return;

		Update(pDeltaTime);

		for (unique<UIElement>& child : _children)
		{
			if (!child) continue;
			child->Dream(pDeltaTime);
		}
	}

	bool UIElement::IsPointInside(float pX, float pY) const
	{
		if (!_layout.HasClip()) return false;
		if (pX < _layout.clipMinX || pY < _layout.clipMinY || pX >= _layout.clipMaxX || pY >= _layout.clipMaxY) return false;

		const float centerX = _layout.rect.x + _layout.rect.width * 0.5f;
		const float centerY = _layout.rect.y + _layout.rect.height * 0.5f;
		float deltaX = pX - centerX;
		float deltaY = pY - centerY;

		if (_transform.rotation != 0.f)
		{
			const float sine = std::sin(-_transform.rotation);
			const float cosine = std::cos(-_transform.rotation);
			const float rotatedX = deltaX * cosine - deltaY * sine;
			const float rotatedY = deltaX * sine + deltaY * cosine;
			deltaX = rotatedX;
			deltaY = rotatedY;
		}

		return std::abs(deltaX) <= _layout.rect.width * 0.5f && std::abs(deltaY) <= _layout.rect.height * 0.5f;
	}

	UIElement* UIElement::HitTest(float pX, float pY)
	{
		if (!_visible || !_enabled) return nullptr;

		if (_clipChildren)
		{
			if (pX < _layout.clipMinX || pY < _layout.clipMinY || pX >= _layout.clipMaxX || pY >= _layout.clipMaxY) return nullptr;
		}

		for (uint32_t i = static_cast<uint32_t>(_children.size()); i > 0; i--)
		{
			UIElement* child = _children[i - 1].get();
			if (!child) continue;

			if (UIElement* hit = child->HitTest(pX, pY)) return hit;
		}

		if (_receivesPointer && IsPointInside(pX, pY)) return this;
		return nullptr;
	}

	void UIElement::Layout(const UIRect& pParent, float pClipMinX, float pClipMinY, float pClipMaxX, float pClipMaxY, float pScale)
	{
		_canvasScale = pScale;

		const float anchorMinX = pParent.x + pParent.width * _transform.anchorMin[0];
		const float anchorMinY = pParent.y + pParent.height * _transform.anchorMin[1];
		const float anchorMaxX = pParent.x + pParent.width * _transform.anchorMax[0];
		const float anchorMaxY = pParent.y + pParent.height * _transform.anchorMax[1];

		//unity sizeDelta: the anchor span plus the authored size, scaled from reference pixels
		float width = (anchorMaxX - anchorMinX) + _transform.size[0] * pScale;
		float height = (anchorMaxY - anchorMinY) + _transform.size[1] * pScale;
		width = ClampSize(width, _transform.minSize[0] * pScale, _transform.maxSize[0] > 0.f ? _transform.maxSize[0] * pScale : 0.f);
		height = ClampSize(height, _transform.minSize[1] * pScale, _transform.maxSize[1] > 0.f ? _transform.maxSize[1] * pScale : 0.f);

		const float pivotX = anchorMinX + (anchorMaxX - anchorMinX) * _transform.pivot[0] + _transform.position[0] * pScale;
		const float pivotY = anchorMinY + (anchorMaxY - anchorMinY) * _transform.pivot[1] + _transform.position[1] * pScale;

		_layout.rect.x = pivotX - width * _transform.pivot[0];
		_layout.rect.y = pivotY - height * _transform.pivot[1];
		_layout.rect.width = width;
		_layout.rect.height = height;

		if (_clipChildren)
		{
			const UIRect clip = IntersectClip(pClipMinX, pClipMinY, pClipMaxX, pClipMaxY, _layout.rect);
			_layout.clipMinX = clip.x;
			_layout.clipMinY = clip.y;
			_layout.clipMaxX = clip.Right();
			_layout.clipMaxY = clip.Bottom();
		}
		else
		{
			_layout.clipMinX = pClipMinX;
			_layout.clipMinY = pClipMinY;
			_layout.clipMaxX = pClipMaxX;
			_layout.clipMaxY = pClipMaxY;
		}

		ArrangeChildren();

		for (unique<UIElement>& child : _children)
		{
			if (!child) continue;
			child->Layout(_layout.rect, _layout.clipMinX, _layout.clipMinY, _layout.clipMaxX, _layout.clipMaxY, pScale);
		}
	}


	void UIElement::LayoutAsRoot(const UIRect& pRect, float pScale)
	{
		_canvasScale = pScale;
		_layout.rect = pRect;
		_layout.clipMinX = pRect.x;
		_layout.clipMinY = pRect.y;
		_layout.clipMaxX = pRect.Right();
		_layout.clipMaxY = pRect.Bottom();

		ArrangeChildren();

		for (unique<UIElement>& child : _children)
		{
			if (!child) continue;
			child->Layout(_layout.rect, _layout.clipMinX, _layout.clipMinY, _layout.clipMaxX, _layout.clipMaxY, pScale);
		}
	}

	void UIElement::CollectQuads(std::vector<UIDrawQuad>& pQuads, float pParentOpacity)
	{
		if (!_visible) return;

		const float opacity = pParentOpacity * _opacity;
		Emit(pQuads, opacity);

		for (unique<UIElement>& child : _children)
		{
			if (!child) continue;
			child->CollectQuads(pQuads, opacity);
		}
	}

	UIWidgetRecord UIElement::ToRecord() const
	{
		UIWidgetRecord record;
		record.type = GetWidgetType();
		record.name = _name;
		record.anchorMin[0] = _transform.anchorMin[0];
		record.anchorMin[1] = _transform.anchorMin[1];
		record.anchorMax[0] = _transform.anchorMax[0];
		record.anchorMax[1] = _transform.anchorMax[1];
		record.pivot[0] = _transform.pivot[0];
		record.pivot[1] = _transform.pivot[1];
		record.position[0] = _transform.position[0];
		record.position[1] = _transform.position[1];
		record.size[0] = _transform.size[0];
		record.size[1] = _transform.size[1];
		record.minSize[0] = _transform.minSize[0];
		record.minSize[1] = _transform.minSize[1];
		record.maxSize[0] = _transform.maxSize[0];
		record.maxSize[1] = _transform.maxSize[1];
		record.rotation = _transform.rotation;
		record.visible = _visible ? 1 : 0;
		record.enabled = _enabled ? 1 : 0;
		FillRecord(record);
		return record;
	}
}
