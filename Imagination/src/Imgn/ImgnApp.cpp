#include "pch.hpp"
#include "ImgnApp.hpp"
#include "ImgnTime.h"

namespace Imgn
{
	void ImgnApp::Run()
	{
		while (_running)
		{
			auto now = std::chrono::steady_clock::now();
			Time deltaTime(now - _lastFrameTime);
			_lastFrameTime = now;

			_entity.Dream(deltaTime);
			_window->Dream();

			if (_renderer->StartFrame())
			{
				_renderer->ExecuteGraph();
				_renderer->EndFrame(_renderer->MakeImageKey("G-BufferAlbedo", _window->GetWidth(), _window->GetHeight()));
			}
		}
	}

	void ImgnApp::OnEvent(Event& pEvent)
	{
		EventDispatcher dispatcher(pEvent);
	}
}