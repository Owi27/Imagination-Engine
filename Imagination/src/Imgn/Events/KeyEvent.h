#pragma once
#include "Event.h"

class KeyEvent : public Event
{
protected:
	int _keyCode;

	KeyEvent(int pKeyCode)
	{
		_keyCode = pKeyCode;
	}

public:
	inline int GetKeyCode() const { return _keyCode; }

	EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
};

class KeyPressedEvent : public KeyEvent
{
	int _repeatCount;

public:
	KeyPressedEvent(int pKeyCode, int pRepeatCount) : KeyEvent(pKeyCode)
	{
		_keyCode = pKeyCode;
		_repeatCount = pRepeatCount;
	}

	inline int GetRepeatCount() const { return _repeatCount; }

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "KeyPressedEvent: " << _keyCode << " (" << _repeatCount << " repeats)";
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyPressed)
};

class KeyReleasedEvent : public KeyEvent
{
public:
	KeyReleasedEvent(int pKeyCode) : KeyEvent(pKeyCode)
	{
		_keyCode = pKeyCode;
	}

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "KeyReleasedEvent: " << _keyCode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyReleased)
};

class KeyTypedEvent : public KeyEvent
{
public:
	KeyTypedEvent(int pKeyCode) : KeyEvent(pKeyCode)
	{
		_keyCode = pKeyCode;
	}

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "KeyTypedEvent: " << _keyCode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyTyped)
};