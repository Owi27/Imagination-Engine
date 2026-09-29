#include "pch.hpp"
#include "Imgn/UI/UIControls.h"

#include "Imgn/UI/UIFont.h"

namespace Imgn
{
	void UIEmitQuad(std::vector<UIDrawQuad>& pQuads, const UILayoutResult& pLayout, float pX, float pY, float pWidth, float pHeight, const float pUV[4], vec4 pColor, float pOpacity, float pRotation, int32_t pTextureIndex, float pFlags)
	{
		if (pWidth <= 0.f || pHeight <= 0.f) return;
		if (!pLayout.HasClip()) return;

		UIDrawQuad quad;
		quad.gpu.rect[0] = pX;
		quad.gpu.rect[1] = pY;
		quad.gpu.rect[2] = pWidth;
		quad.gpu.rect[3] = pHeight;
		quad.gpu.uv[0] = pUV ? pUV[0] : 0.f;
		quad.gpu.uv[1] = pUV ? pUV[1] : 0.f;
		quad.gpu.uv[2] = pUV ? pUV[2] : 1.f;
		quad.gpu.uv[3] = pUV ? pUV[3] : 1.f;
		quad.gpu.color[0] = pColor[0];
		quad.gpu.color[1] = pColor[1];
		quad.gpu.color[2] = pColor[2];
		quad.gpu.color[3] = pColor[3] * pOpacity;
		quad.gpu.extra[0] = static_cast<float>(pTextureIndex);
		quad.gpu.extra[1] = pFlags;
		quad.gpu.extra[2] = pRotation;
		quad.gpu.extra[3] = 0.f;
		quad.clipMinX = pLayout.clipMinX;
		quad.clipMinY = pLayout.clipMinY;
		quad.clipMaxX = pLayout.clipMaxX;
		quad.clipMaxY = pLayout.clipMaxY;

		if (quad.gpu.color[3] <= 0.f) return;
		pQuads.push_back(quad);
	}

	void UIPanel::Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity)
	{
		const float uv[4] = { 0.f, 0.f, 1.f, 1.f };
		UIEmitQuad(pQuads, GetLayout(), GetLayout().rect.x, GetLayout().rect.y, GetLayout().rect.width, GetLayout().rect.height, uv, _color, pOpacity, GetTransform().rotation, UISolidTexture, 0.f);
	}

	void UIPanel::FillRecord(UIWidgetRecord& pRecord) const
	{
		pRecord.color[0] = _color[0];
		pRecord.color[1] = _color[1];
		pRecord.color[2] = _color[2];
		pRecord.color[3] = _color[3];
	}

	void UIImage::Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity)
	{
		UIEmitQuad(pQuads, GetLayout(), GetLayout().rect.x, GetLayout().rect.y, GetLayout().rect.width, GetLayout().rect.height, _uv, _color, pOpacity, GetTransform().rotation, _textureIndex, 0.f);
	}

	void UIImage::FillRecord(UIWidgetRecord& pRecord) const
	{
		pRecord.color[0] = _color[0];
		pRecord.color[1] = _color[1];
		pRecord.color[2] = _color[2];
		pRecord.color[3] = _color[3];
		pRecord.value = static_cast<float>(_textureIndex);
	}

	void UIText::Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity)
	{
		if (_text.empty()) return;

		const float scale = GetCanvasScale() > 0.f ? GetCanvasScale() : 1.f;
		const float glyph = _fontSize * scale;
		if (glyph <= 0.f) return;

		uint32_t lines = 1;
		uint32_t column = 0;
		uint32_t widest = 0;
		for (char character : _text)
		{
			if (character == '\n')
			{
				if (column > widest) widest = column;
				column = 0;
				lines++;
				continue;
			}
			column++;
		}
		if (column > widest) widest = column;

		const float blockWidth = static_cast<float>(widest) * glyph;
		const float blockHeight = static_cast<float>(lines) * glyph;
		float originX = GetLayout().rect.x;
		if (_alignment == eUITextAlignment::Center) originX += (GetLayout().rect.width - blockWidth) * 0.5f;
		if (_alignment == eUITextAlignment::Right) originX += GetLayout().rect.width - blockWidth;
		float originY = GetLayout().rect.y + (GetLayout().rect.height - blockHeight) * 0.5f;

		float penX = originX;
		float penY = originY;
		UIFont* font = UIFontManager::Get().Find(_font);

		for (unsigned char character : _text)
		{
			if (character == '\n')
			{
				penX = originX;
				penY += glyph;
				continue;
			}

			UIGlyph glyphInfo;
			if (font && font->TryGetGlyph(character, glyphInfo))
			{
				const float uv[4] = { glyphInfo.u0, glyphInfo.v0, glyphInfo.u1, glyphInfo.v1 };
				UIEmitQuad(pQuads, GetLayout(), penX, penY, glyph, glyph, uv, _color, pOpacity, 0.f, font->GetTextureIndex(), 0.f);
			}

			penX += glyph;
		}
	}

	void UIText::FillRecord(UIWidgetRecord& pRecord) const
	{
		pRecord.text = _text;
		pRecord.color[0] = _color[0];
		pRecord.color[1] = _color[1];
		pRecord.color[2] = _color[2];
		pRecord.color[3] = _color[3];
		pRecord.value = _fontSize;
	}

	vec4 UIButton::ColorForState() const
	{
		if (!IsEnabled() || _state == eUIState::Disabled) return _style.disabled;

		switch (_state)
		{
			case eUIState::Hovered: return _style.hovered;
			case eUIState::Pressed: return _style.pressed;
			case eUIState::Focused: return _style.focused;
			case eUIState::Disabled: return _style.disabled;
			case eUIState::Normal: return _style.normal;
		}

		return _style.normal;
	}

	void UIButton::Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity)
	{
		const float uv[4] = { 0.f, 0.f, 1.f, 1.f };
		UIEmitQuad(pQuads, GetLayout(), GetLayout().rect.x, GetLayout().rect.y, GetLayout().rect.width, GetLayout().rect.height, uv, ColorForState(), pOpacity, GetTransform().rotation, UISolidTexture, 0.f);
	}

	void UIButton::FillRecord(UIWidgetRecord& pRecord) const
	{
		if (_label) pRecord.text = _label->GetText();
		const vec4 color = ColorForState();
		pRecord.color[0] = color[0];
		pRecord.color[1] = color[1];
		pRecord.color[2] = color[2];
		pRecord.color[3] = color[3];
	}

	UIButton::UIButton()
	{
		SetName("Button");
		_label = AddChild<UIText>();
		_label->SetName("Label");
		_label->SetText("Button");
		_label->SetAlignment(eUITextAlignment::Center);
		_label->SetFontSize(28.f);
		_label->GetTransform().anchorMin = { 0.f, 0.f };
		_label->GetTransform().anchorMax = { 1.f, 1.f };
		_label->GetTransform().pivot = { 0.5f, 0.5f };
		_label->GetTransform().position = { 0.f, 0.f };
		_label->GetTransform().size = { 0.f, 0.f };
	}

	void UIButton::SetText(const std::string& pText)
	{
		if (_label) _label->SetText(pText);
	}

	void UIButton::OnPointerEnter()
	{
		if (!IsEnabled()) { _state = eUIState::Disabled; return; }
		if (_state != eUIState::Pressed) _state = eUIState::Hovered;
	}

	void UIButton::OnPointerExit()
	{
		if (!IsEnabled()) { _state = eUIState::Disabled; return; }
		_state = eUIState::Normal;
	}

	void UIButton::OnPointerDown(float pX, float pY)
	{
		(void)pX; (void)pY;
		if (!IsEnabled()) return;
		_state = eUIState::Pressed;
	}

	void UIButton::OnPointerUp(float pX, float pY)
	{
		(void)pX; (void)pY;
		if (!IsEnabled()) return;
		_state = eUIState::Hovered;
	}

	void UIProgressBar::SetValue(float pValue)
	{
		if (pValue < 0.f) pValue = 0.f;
		if (pValue > 1.f) pValue = 1.f;
		_value = pValue;
	}

	void UIProgressBar::Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity)
	{
		const float uv[4] = { 0.f, 0.f, 1.f, 1.f };
		const UIRect& rect = GetLayout().rect;
		UIEmitQuad(pQuads, GetLayout(), rect.x, rect.y, rect.width, rect.height, uv, _track, pOpacity, 0.f, UISolidTexture, 0.f);

		const float fill = rect.width * _value;
		if (fill <= 0.f) return;
		UIEmitQuad(pQuads, GetLayout(), rect.x, rect.y, fill, rect.height, uv, _fill, pOpacity, 0.f, UISolidTexture, 0.f);
	}

	void UIProgressBar::FillRecord(UIWidgetRecord& pRecord) const
	{
		pRecord.value = _value;
		pRecord.color[0] = _fill[0];
		pRecord.color[1] = _fill[1];
		pRecord.color[2] = _fill[2];
		pRecord.color[3] = _fill[3];
	}

	void UISlider::SetValue(float pValue)
	{
		if (pValue < 0.f) pValue = 0.f;
		if (pValue > 1.f) pValue = 1.f;
		_value = pValue;
	}

	void UISlider::SetFromPointer(float pX)
	{
		const float width = GetLayout().rect.width;
		if (width <= 0.f) return;
		SetValue((pX - GetLayout().rect.x) / width);
	}

	void UISlider::Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity)
	{
		const float uv[4] = { 0.f, 0.f, 1.f, 1.f };
		const UIRect& rect = GetLayout().rect;
		const float trackHeight = rect.height * 0.35f;
		const float trackY = rect.y + (rect.height - trackHeight) * 0.5f;
		UIEmitQuad(pQuads, GetLayout(), rect.x, trackY, rect.width, trackHeight, uv, _track, pOpacity, 0.f, UISolidTexture, 0.f);

		const float knob = rect.height;
		const float knobX = rect.x + rect.width * _value - knob * 0.5f;
		UIEmitQuad(pQuads, GetLayout(), knobX, rect.y, knob, knob, uv, _knob, pOpacity, 0.f, UISolidTexture, 0.f);
	}

	void UISlider::FillRecord(UIWidgetRecord& pRecord) const
	{
		pRecord.value = _value;
	}

	void UISlider::OnPointerDown(float pX, float pY)
	{
		(void)pY;
		SetFromPointer(pX);
	}

	void UISlider::OnDrag(float pX, float pY, float pDeltaX, float pDeltaY)
	{
		(void)pY; (void)pDeltaX; (void)pDeltaY;
		SetFromPointer(pX);
	}

	UICheckbox::UICheckbox()
	{
		SetName("Checkbox");
		_label = AddChild<UIText>();
		_label->SetName("Label");
		_label->SetAlignment(eUITextAlignment::Left);
		_label->SetFontSize(28.f);
		_label->GetTransform().anchorMin = { 0.f, 0.f };
		_label->GetTransform().anchorMax = { 1.f, 1.f };
		_label->GetTransform().pivot = { 0.f, 0.5f };
		_label->GetTransform().position = { 36.f, 0.f };
		_label->GetTransform().size = { -36.f, 0.f };
	}

	void UICheckbox::SetText(const std::string& pText)
	{
		if (_label) _label->SetText(pText);
	}

	void UICheckbox::Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity)
	{
		const float uv[4] = { 0.f, 0.f, 1.f, 1.f };
		const UIRect& rect = GetLayout().rect;
		const float box = rect.height < rect.width ? rect.height : rect.width;
		const float size = box > 28.f * GetCanvasScale() ? 28.f * GetCanvasScale() : box;
		const float y = rect.y + (rect.height - size) * 0.5f;
		UIEmitQuad(pQuads, GetLayout(), rect.x, y, size, size, uv, _box, pOpacity, 0.f, UISolidTexture, 0.f);

		if (!_checked) return;
		const float inset = size * 0.22f;
		UIEmitQuad(pQuads, GetLayout(), rect.x + inset, y + inset, size - inset * 2.f, size - inset * 2.f, uv, _mark, pOpacity, 0.f, UISolidTexture, 0.f);
	}

	void UICheckbox::FillRecord(UIWidgetRecord& pRecord) const
	{
		pRecord.value = _checked ? 1.f : 0.f;
		if (_label) pRecord.text = _label->GetText();
	}

	void UICheckbox::OnClick()
	{
		if (!IsEnabled()) return;
		_checked = !_checked;
	}

	void UIScrollView::SetScroll(float pScroll)
	{
		if (pScroll < 0.f) pScroll = 0.f;
		_scroll = pScroll;
	}

	void UIScrollView::ArrangeChildren()
	{
		const float scale = GetCanvasScale() > 0.f ? GetCanvasScale() : 1.f;
		float cursor = -_scroll / scale;

		for (const unique<UIElement>& child : GetChildren())
		{
			if (!child) continue;

			UITransform& transform = child->GetTransform();
			transform.anchorMin = { 0.f, 0.f };
			transform.anchorMax = { 1.f, 0.f };
			transform.pivot = { 0.f, 0.f };
			transform.position = { 0.f, cursor };

			float height = transform.size[1];
			if (height <= 0.f) height = 32.f;
			transform.size[1] = height;
			//width stretches with the view; size x is extra padding
			cursor += height + _spacing;
		}

		_contentHeight = (cursor + _scroll / scale) * scale;
	}

	void UIScrollView::FillRecord(UIWidgetRecord& pRecord) const
	{
		pRecord.value = _scroll;
	}

	void UIScrollView::OnScroll(float pDelta)
	{
		const float limit = _contentHeight - GetLayout().rect.height;
		_scroll -= pDelta * 48.f;
		if (_scroll < 0.f) _scroll = 0.f;
		if (limit > 0.f && _scroll > limit) _scroll = limit;
	}
}
