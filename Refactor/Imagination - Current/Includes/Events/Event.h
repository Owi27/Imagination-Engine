#pragma once
#include "pch.h"
//#include "Imagination/Core.h"
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

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; } virtual EventType GetEventType() const override { return GetStaticType(); } virtual const char* GetName() const override { return #type; } virtual Event* Clone() const override { return new type##Event(*this); }
#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

class Event
{
	friend class EventDispatcher;

protected:
	bool _handled = false;

public:
	virtual Event* Clone() const = 0;
	virtual const char* GetName() const = 0;
	virtual int GetCategoryFlags() const = 0;
	virtual EventType GetEventType() const = 0;
	virtual std::string ToString() const { return GetName(); }

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

class EventBus
{
	std::vector<EventListener*> _listeners;
	std::queue<std::unique_ptr<Event>> _eventQueue;
	std::mutex _queueMutex;
	bool _immediateMode = true;

public:
	void SetImmediateMode(bool pImmediate)
	{
		_immediateMode = pImmediate;
	}

	void AddListener(EventListener* pListener)
	{
		_listeners.push_back(pListener);
	}

	void RemoveListener(EventListener* pListener)
	{
		auto it = std::find(_listeners.begin(), _listeners.end(), pListener);
		if (it != _listeners.end()) _listeners.erase(it);
	}

	void PublishEvent(Event& pEvent)
	{
		if (_immediateMode)
		{
			// Dispatch event immediately
			for (auto listener : _listeners)
			{
				listener->OnEvent(pEvent);
			}
		}
		else
		{
			// Queue event for later processing
			std::lock_guard<std::mutex> lock(_queueMutex);
			_eventQueue.push(std::unique_ptr<Event>(pEvent.Clone()));
		}
	}

	void ProcessEvents()
	{
		if (_immediateMode) return;

		std::queue<std::unique_ptr<Event>> currentEvents;

		{
			std::lock_guard<std::mutex> lock(_queueMutex);
			std::swap(currentEvents, _eventQueue);
		}

		while (!currentEvents.empty())
		{
			auto& event = *currentEvents.front();

			for (auto listener : _listeners)
			{
				listener->OnEvent(event);
			}

			currentEvents.pop();
		}
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