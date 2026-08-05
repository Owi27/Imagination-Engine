#include "pch.hpp"
#include "ImgnApp.hpp"

namespace Imgn
{
	void ImgnApp::Run()
	{
		while (_running)
		{
			_entity.Dream(1.f);
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