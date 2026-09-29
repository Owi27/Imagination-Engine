#include "pch.hpp"
#include "Imgn/UI/UIInput.h"

#include "Imgn/UI/UICanvas.h"
#include "Imgn/UI/UIControls.h"

namespace Imgn
{
	UIElement* UIInputSystem::Hit(const std::vector<UICanvas*>& pCanvases) const
	{
		for (uint32_t i = static_cast<uint32_t>(pCanvases.size()); i > 0; i--)
		{
			UICanvas* canvas = pCanvases[i - 1];
			if (!canvas || !canvas->IsVisible() || !canvas->IsEnabled()) continue;
			if (canvas->GetMode() != eUICanvasMode::ScreenSpace) continue;
			if (UIElement* hit = canvas->HitTest(_frame.x, _frame.y)) return hit;
		}
		return nullptr;
	}

	void UIInputSystem::Apply(const std::vector<UICanvas*>& pCanvases)
	{
		if (!_frame.active)
		{
			if (_hovered)
			{
				_hovered->OnPointerExit();
				_hovered = nullptr;
			}
			_pressed = nullptr;
			_frame.pressed = false;
			_frame.released = false;
			_frame.scroll = 0.f;
			return;
		}

		UIElement* hit = Hit(pCanvases);

		if (hit != _hovered)
		{
			if (_hovered) _hovered->OnPointerExit();
			_hovered = hit;
			if (_hovered) _hovered->OnPointerEnter();
		}

		if (_frame.pressed && hit)
		{
			_pressed = hit;
			_focused = hit;
			hit->OnPointerDown(_frame.x, _frame.y);
		}

		if (_pressed && (_frame.x != _lastX || _frame.y != _lastY) && _frame.down)
			_pressed->OnDrag(_frame.x, _frame.y, _frame.x - _lastX, _frame.y - _lastY);

		if (_frame.released)
		{
			if (_pressed) _pressed->OnPointerUp(_frame.x, _frame.y);
			if (_pressed && _pressed == hit)
			{
				_pressed->OnClick();
				if (UIButton* button = dynamic_cast<UIButton*>(_pressed))
				{
					if (button->IsEnabled() && button->OnClick) button->OnClick();
				}
			}
			_pressed = nullptr;
		}

		if (hit && _frame.scroll != 0.f)
		{
			UIElement* scrollTarget = hit;
			while (scrollTarget && scrollTarget->GetWidgetType() != eUIWidgetType::ScrollView)
				scrollTarget = scrollTarget->GetParent();
			if (scrollTarget) scrollTarget->OnScroll(_frame.scroll);
		}

		_lastX = _frame.x;
		_lastY = _frame.y;
		_frame.pressed = false;
		_frame.released = false;
		_frame.scroll = 0.f;
	}
}
