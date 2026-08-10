#pragma once
//#include "pch.h"
#include "ImgnCore.hpp"
//#include <spdlog/fmt/fmt.h>

#define BIT(x) (1 << x)

enum class EventType
{
	None,
	WindowClosed,
	WindowResized,
	WindowFocused,
	WindowLostFocus,
	WindowMoved,
	AppTick,
	AppUpdate,
	AppRender,
	KeyPressed,
	KeyReleased,
	KeyTyped,
	MouseButtonPressed,
	MouseButtonReleased,
	MouseMoved,
	MouseScrolled
};

enum EventCategory
{
	None,
	EventCategoryApplication = BIT(0),
	EventCategoryInput = BIT(1),
	EventCategoryKeyboard = BIT(2),
	EventCategoryMouse = BIT(3),
	EventCategoryMouseButton = BIT(4)
};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; } virtual EventType GetEventType() const override { return GetStaticType(); } virtual const char* GetName() const override { return #type; }
#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

class Event
{
	friend class EventDispatcher;

protected:
	bool _handled = false;

public:
	virtual const char* GetName() const = 0;
	virtual int GetCategoryFlags() const = 0;
	virtual EventType GetEventType() const = 0;
	virtual std::string ToString() const { return GetName(); }
	virtual bool Handled() const { return _handled; }

	inline bool IsInCategory(EventCategory category)
	{
		return GetCategoryFlags() & category;
	}
};

class EventListener
{
public:
	virtual ~EventListener() = default;
	virtual void OnEvent(Event& event) = 0;
};

class EventDispatcher
{
	template<typename T>
	using EventFunction = std::function<bool(T&)>;

	Event& _event;

public:
	EventDispatcher(Event& pEvent) : _event(pEvent)
	{
	}

	//void SetEvent(Event& pEvent) { _event = pEvent; }

	template<typename T>
	bool Dispatch(EventFunction<T> pFunc)
	{
		if (_event.GetEventType() == T::GetStaticType())
		{
			_event._handled = pFunc(*(T*)&_event);
			return true;
		}

		return false;
	}
};

inline std::ostream& operator<<(std::ostream& pOS, const Event& pE)
{
	return pOS << pE.ToString();
}
//
//template<>
//struct fmt::formatter<Imagination::Event>
//{
//	constexpr auto parse(fmt::format_parse_context& pCtx) { return pCtx.begin(); }
//
//	auto format(const Imagination::Event& pE, fmt::format_context& pCtx) const { return fmt::format_to(pCtx.out(), "{}", pE.ToString()); }
//};