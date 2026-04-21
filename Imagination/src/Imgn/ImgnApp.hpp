#pragma once
#include "ImgnCore.hpp"
#include "ImgnWindow.h"
#include "Events/Event.h"
#include "ImgnComponent.h"

class IMGN_API ImgnApp
{
	bool _running = true;
	unique<ImgnWindow> _window;
	ImgnEntity _entity;

public:
	ImgnApp()
	{
		_window = std::make_unique<ImgnWindow>();
	}

	~ImgnApp()
	{

	}

	void Run();
	void OnEvent(Event& pEvent);

	template<typename T, typename... Args>
	T* AddComponent(Args&&... pArgs)
	{
		return _entity.AddComponent<T>(std::forward<Args>(pArgs)...);
	}

	template<typename T>
	void RemoveComponent()
	{
		_entity.RemoveComponent<T>();
	}
};

namespace IMGN
{
	ImgnApp* CreateApplication();
}