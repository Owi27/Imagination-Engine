#pragma once
#include "Event.h"

class WindowResizedEvent : public Event
{
	unsigned _width, _height;

public:
	WindowResizedEvent(unsigned pWidth, unsigned pHeight) /*Constructor*/
	{
		_width = pWidth;
		_height = pHeight;
	}

	inline unsigned GetWidth() const { return _width; }
	inline unsigned GetHeight() const { return _height; }

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "WindowResizeEvent: " << _width << ", " << _height;
		return ss.str();
	}

	EVENT_CLASS_TYPE(WindowResized)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class WindowClosedEvent : public Event
{
public:
	WindowClosedEvent()
	{
	}

	EVENT_CLASS_TYPE(WindowClosed)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppTickEvent : public Event
{
public:
	AppTickEvent()
	{
	}

	EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppUpdateEvent : public Event
{
public:
	AppUpdateEvent()
	{
	}

	EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppRenderEvent : public Event
{
public:
	AppRenderEvent()
	{
	}

	EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
};