#include "pch.hpp"
#include "ImgnApp.hpp"

void ImgnApp::Run()
{
	while (_running)
	{
		_entity.Dream(1.f);
		_window->Dream();
	}
}

void ImgnApp::OnEvent(Event& pEvent)
{
	EventDispatcher dispatcher(pEvent);
}