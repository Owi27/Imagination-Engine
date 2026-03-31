//#pragma once
//class Event
//{
//public:
//	virtual ~Event() = default;
//};
//
//class EventListener
//{
//public:
//	~EventListener() = default;
//	virtual void OnEvent(const Event& pEvent) = 0;
//};
//
//class EventSystem
//{
//private:
//    std::vector<EventListener*> _listeners;
//
//public:
//    void AddListener(EventListener* listener)
//    {
//        _listeners.push_back(listener);
//    }
//
//    void RemoveListener(EventListener* listener)
//    {
//        auto it = std::find(_listeners.begin(), _listeners.end(), listener);
//        if (it != _listeners.end())
//        {
//            _listeners.erase(it);
//        }
//    }
//
//    void DispatchEvent(const Event& event)
//    {
//        for (auto listener : _listeners)
//        {
//            listener->OnEvent(event);
//        }
//    }
//};
//
