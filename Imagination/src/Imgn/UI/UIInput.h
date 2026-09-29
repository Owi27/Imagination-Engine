#pragma once
#include "Imgn/UI/UIElement.h"

namespace Imgn
{
	class UICanvas;

	struct UIPointerFrame
	{
		bool active = false;
		float x = 0.f, y = 0.f;
		bool down = false;
		bool pressed = false;
		bool released = false;
		float scroll = 0.f;
	};

	class IMGN_API UIInputSystem
	{
		UIElement* _hovered = nullptr;
		UIElement* _pressed = nullptr;
		UIElement* _focused = nullptr;
		float _lastX = 0.f, _lastY = 0.f;
		UIPointerFrame _frame;

		UIElement* Hit(const std::vector<UICanvas*>& pCanvases) const;

	public:
		void SetFrame(const UIPointerFrame& pFrame) { _frame = pFrame; }
		void Apply(const std::vector<UICanvas*>& pCanvases);
		UIElement* GetHovered() const { return _hovered; }
		UIElement* GetFocused() const { return _focused; }
	};
}
