#pragma once
#include "Event.h"

class MouseMovedEvent : public Event
{
	float _mouseX, _mouseY;

public:
	MouseMovedEvent(float pX, float pY)
	{
		_mouseX = pX;
		_mouseY = pY;
	}

	inline float GetX() const { return _mouseX; }
	inline float GetY() const { return _mouseY; }

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseMovedEvent: " << _mouseX << ", " << _mouseY;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
};

class MouseScrolledEvent : public Event
{
	float _xOffset, _yOffset;

public:
	MouseScrolledEvent(float pX, float pY)
	{
		_xOffset = pX;
		_yOffset = pY;
	}

	inline float GetXOffset() const { return _xOffset; }
	inline float GetYOffset() const { return _yOffset; }

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseScrolledEvent: " << _xOffset << ", " << _yOffset;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
};

class MouseButtonEvent : public Event
{
protected:
	int _button;

	MouseButtonEvent(int pButton)
	{
		_button = pButton;
	}

public:
	inline int GetMouseButton() const { return _button; }

	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
};

class MouseButtonPressedEvent : public MouseButtonEvent
{
public:
	MouseButtonPressedEvent(int pButton) : MouseButtonEvent(pButton)
	{
		_button = pButton;
	}

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseButtonPressedEvent: " << _button;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
	MouseButtonReleasedEvent(int pButton) : MouseButtonEvent(pButton)
	{
		_button = pButton;
	}

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseButtonReleasedEvent: " << _button;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseButtonReleased)
};