#pragma once
#include "Imgn/UI/UIElement.h"

namespace Imgn
{
	class IMGN_API UICanvas : public UIElement
	{
		eUICanvasMode _mode = eUICanvasMode::ScreenSpace;
		uint32_t _referenceWidth = UIReferenceWidth;
		uint32_t _referenceHeight = UIReferenceHeight;
		uint32_t _pixelWidth = UIReferenceWidth;
		uint32_t _pixelHeight = UIReferenceHeight;
		float _scale = 1.f;

	public:
		UICanvas()
		{
			SetName("Canvas");
			SetReceivesPointer(false);
			GetTransform().anchorMin = { 0.f, 0.f };
			GetTransform().anchorMax = { 0.f, 0.f };
			GetTransform().pivot = { 0.f, 0.f };
			GetTransform().position = { 0.f, 0.f };
			GetTransform().size = { static_cast<float>(UIReferenceWidth), static_cast<float>(UIReferenceHeight) };
		}

		eUIWidgetType GetWidgetType() const override { return eUIWidgetType::Element; }

		void SetMode(eUICanvasMode pMode) { _mode = pMode; }
		eUICanvasMode GetMode() const { return _mode; }

		void SetReferenceResolution(uint32_t pWidth, uint32_t pHeight)
		{
			if (pWidth == 0 || pHeight == 0) return;
			_referenceWidth = pWidth;
			_referenceHeight = pHeight;
		}

		uint32_t GetReferenceWidth() const { return _referenceWidth; }
		uint32_t GetReferenceHeight() const { return _referenceHeight; }
		uint32_t GetPixelWidth() const { return _pixelWidth; }
		uint32_t GetPixelHeight() const { return _pixelHeight; }
		float GetScale() const { return _scale; }

		//screen pixels. world-space is intentionally not implemented.
		void Resize(uint32_t pWidth, uint32_t pHeight);
		UIRect ContentRect() const;
	};
}
