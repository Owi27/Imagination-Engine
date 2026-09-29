#pragma once
#include "Imgn/UI/UIElement.h"

#include <functional>

namespace Imgn
{
	class IMGN_API UIPanel : public UIElement
	{
		vec4 _color = { 0.08f, 0.09f, 0.11f, 0.86f };

	protected:
		void Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity) override;
		void FillRecord(UIWidgetRecord& pRecord) const override;

	public:
		UIPanel() { SetName("Panel"); }

		void SetColor(vec4 pColor) { _color = pColor; }
		vec4 GetColor() const { return _color; }
		eUIWidgetType GetWidgetType() const override { return eUIWidgetType::Panel; }
	};

	class IMGN_API UIImage : public UIElement
	{
		vec4 _color = { 1.f, 1.f, 1.f, 1.f };
		float _uv[4] = { 0.f, 0.f, 1.f, 1.f };
		int32_t _textureIndex = UISolidTexture;

	protected:
		void Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity) override;
		void FillRecord(UIWidgetRecord& pRecord) const override;

	public:
		UIImage() { SetName("Image"); SetReceivesPointer(false); }

		void SetColor(vec4 pColor) { _color = pColor; }
		void SetTextureIndex(int32_t pTextureIndex) { _textureIndex = pTextureIndex; }
		void SetUV(float pU0, float pV0, float pU1, float pV1)
		{
			_uv[0] = pU0; _uv[1] = pV0; _uv[2] = pU1; _uv[3] = pV1;
		}

		int32_t GetTextureIndex() const { return _textureIndex; }
		eUIWidgetType GetWidgetType() const override { return eUIWidgetType::Image; }
	};

	class IMGN_API UIText : public UIElement
	{
		std::string _text;
		std::string _font = "Default";
		float _fontSize = 32.f;
		vec4 _color = { 1.f, 1.f, 1.f, 1.f };
		eUITextAlignment _alignment = eUITextAlignment::Left;

	protected:
		void Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity) override;
		void FillRecord(UIWidgetRecord& pRecord) const override;

	public:
		UIText()
		{
			SetName("Text");
			SetReceivesPointer(false);
		}

		void SetText(const std::string& pText) { _text = pText; }
		const std::string& GetText() const { return _text; }
		void SetFont(const std::string& pFont) { _font = pFont; }
		const std::string& GetFont() const { return _font; }
		void SetFontSize(float pSize) { _fontSize = pSize; }
		float GetFontSize() const { return _fontSize; }
		void SetColor(vec4 pColor) { _color = pColor; }
		void SetAlignment(eUITextAlignment pAlignment) { _alignment = pAlignment; }
		eUIWidgetType GetWidgetType() const override { return eUIWidgetType::Text; }
	};

	class IMGN_API UIButton : public UIElement
	{
		UIButtonStyle _style;
		eUIState _state = eUIState::Normal;
		UIText* _label = nullptr;

		vec4 ColorForState() const;

	protected:
		void Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity) override;
		void FillRecord(UIWidgetRecord& pRecord) const override;

	public:
		std::function<void()> OnClick;

		UIButton();

		void SetText(const std::string& pText);
		void SetStyle(const UIButtonStyle& pStyle) { _style = pStyle; }
		eUIState GetState() const { return _state; }
		eUIWidgetType GetWidgetType() const override { return eUIWidgetType::Button; }

		void OnPointerEnter() override;
		void OnPointerExit() override;
		void OnPointerDown(float pX, float pY) override;
		void OnPointerUp(float pX, float pY) override;
	};

	class IMGN_API UIProgressBar : public UIElement
	{
		float _value = 0.f;
		vec4 _track = { 0.05f, 0.05f, 0.05f, 0.9f };
		vec4 _fill = { 0.20f, 0.72f, 0.32f, 1.f };

	protected:
		void Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity) override;
		void FillRecord(UIWidgetRecord& pRecord) const override;

	public:
		UIProgressBar() { SetName("ProgressBar"); SetReceivesPointer(false); }

		void SetValue(float pValue);
		float GetValue() const { return _value; }
		void SetTrackColor(vec4 pColor) { _track = pColor; }
		void SetFillColor(vec4 pColor) { _fill = pColor; }
		eUIWidgetType GetWidgetType() const override { return eUIWidgetType::ProgressBar; }
	};

	class IMGN_API UISlider : public UIElement
	{
		float _value = 0.f;
		vec4 _track = { 0.05f, 0.05f, 0.05f, 0.9f };
		vec4 _knob = { 0.85f, 0.85f, 0.85f, 1.f };

		void SetFromPointer(float pX);

	protected:
		void Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity) override;
		void FillRecord(UIWidgetRecord& pRecord) const override;

	public:
		UISlider() { SetName("Slider"); }

		void SetValue(float pValue);
		float GetValue() const { return _value; }
		eUIWidgetType GetWidgetType() const override { return eUIWidgetType::Slider; }

		void OnPointerDown(float pX, float pY) override;
		void OnDrag(float pX, float pY, float pDeltaX, float pDeltaY) override;
	};

	class IMGN_API UICheckbox : public UIElement
	{
		bool _checked = false;
		vec4 _box = { 0.12f, 0.12f, 0.12f, 0.95f };
		vec4 _mark = { 0.30f, 0.85f, 0.45f, 1.f };
		UIText* _label = nullptr;

	protected:
		void Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity) override;
		void FillRecord(UIWidgetRecord& pRecord) const override;

	public:
		UICheckbox();

		void SetChecked(bool pChecked) { _checked = pChecked; }
		bool IsChecked() const { return _checked; }
		void SetText(const std::string& pText);
		eUIWidgetType GetWidgetType() const override { return eUIWidgetType::Checkbox; }
		void OnClick() override;
	};

	class IMGN_API UIScrollView : public UIElement
	{
		float _scroll = 0.f;
		float _spacing = 6.f;
		float _contentHeight = 0.f;

	protected:
		void ArrangeChildren() override;
		void FillRecord(UIWidgetRecord& pRecord) const override;

	public:
		UIScrollView()
		{
			SetName("ScrollView");
			SetClipChildren(true);
		}

		void SetScroll(float pScroll);
		float GetScroll() const { return _scroll; }
		eUIWidgetType GetWidgetType() const override { return eUIWidgetType::ScrollView; }
		void OnScroll(float pDelta) override;
	};

	void UIEmitQuad(std::vector<UIDrawQuad>& pQuads, const UILayoutResult& pLayout, float pX, float pY, float pWidth, float pHeight, const float pUV[4], vec4 pColor, float pOpacity, float pRotation, int32_t pTextureIndex, float pFlags);
}
