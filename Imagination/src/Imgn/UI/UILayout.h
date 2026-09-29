#pragma once
#include "Imgn/UI/UIElement.h"

namespace Imgn
{
	class UICanvas;

	class IMGN_API UILayoutSystem
	{
		UILayoutSystem() = delete;

	public:
		static void CalculateLayout(UICanvas* pCanvas);
	};

	class IMGN_API UIHorizontalLayout : public UIElement
	{
		float _spacing = 8.f;
		float _padding = 8.f;

	protected:
		void ArrangeChildren() override;

	public:
		UIHorizontalLayout()
		{
			SetName("HorizontalLayout");
			SetReceivesPointer(false);
		}

		void SetSpacing(float pSpacing) { _spacing = pSpacing; }
		void SetPadding(float pPadding) { _padding = pPadding; }
		eUIWidgetType GetWidgetType() const override { return eUIWidgetType::HorizontalLayout; }
	};

	class IMGN_API UIVerticalLayout : public UIElement
	{
		float _spacing = 8.f;
		float _padding = 8.f;

	protected:
		void ArrangeChildren() override;

	public:
		UIVerticalLayout()
		{
			SetName("VerticalLayout");
			SetReceivesPointer(false);
		}

		void SetSpacing(float pSpacing) { _spacing = pSpacing; }
		void SetPadding(float pPadding) { _padding = pPadding; }
		eUIWidgetType GetWidgetType() const override { return eUIWidgetType::VerticalLayout; }
	};

	class IMGN_API UIGridLayout : public UIElement
	{
		vec2 _cellSize = { 64.f, 64.f };
		vec2 _spacing = { 8.f, 8.f };
		uint32_t _columns = 2;
		float _padding = 8.f;

	protected:
		void ArrangeChildren() override;

	public:
		UIGridLayout()
		{
			SetName("GridLayout");
			SetReceivesPointer(false);
		}

		void SetCellSize(vec2 pCellSize) { _cellSize = pCellSize; }
		void SetSpacing(vec2 pSpacing) { _spacing = pSpacing; }
		void SetColumns(uint32_t pColumns) { _columns = pColumns < 1 ? 1 : pColumns; }
		void SetPadding(float pPadding) { _padding = pPadding; }
		eUIWidgetType GetWidgetType() const override { return eUIWidgetType::GridLayout; }
	};

	class IMGN_API UIOverlayLayout : public UIElement
	{
	protected:
		void ArrangeChildren() override;

	public:
		UIOverlayLayout()
		{
			SetName("OverlayLayout");
			SetReceivesPointer(false);
		}

		eUIWidgetType GetWidgetType() const override { return eUIWidgetType::OverlayLayout; }
	};
}
