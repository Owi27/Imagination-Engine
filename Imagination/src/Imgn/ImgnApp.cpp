#include "pch.hpp"
#include "ImgnApp.hpp"
#include "ImgnTime.h"
#include "ImgnCamera.h"

namespace Imgn
{
	void ImgnApp::Run()
	{
		while (_running)
		{
			auto now = std::chrono::steady_clock::now();
			Time deltaTime(now - _lastFrameTime);
			_lastFrameTime = now;

			if (_renderer->StartFrame())
			{
				for (unique<Layer>& layer : _layerStack)
				{
					layer->Dream(deltaTime);
				}

				_imGuiLayer->Begin();
				for (unique<Layer>& layer : _layerStack)
				{
					layer->OnImGuiRender();
				}
				_imGuiLayer->OnImGuiRender();
				_imGuiLayer->End();

				_renderer->EndFrame();
			}

			//IMGN_CORE_TRACE("x {}, y {}", Input::GetMouseX(), Input::GetMouseY());

			_entity.Dream(deltaTime);
			_window->Dream();
		}
	}

	void ImgnApp::OnEvent(Event& pEvent)
	{
		EventDispatcher dispatcher(pEvent);

		for (auto iter = _layerStack.end(); iter != _layerStack.begin();)
		{
			(*--iter)->OnEvent(pEvent);
			if (pEvent.Handled()) break;
		}
		
		_imGuiLayer->OnEvent(pEvent);
	}

	void ImgnApp::AddLayer(unique<Layer> pLayer)
	{
		pLayer->Sleep();
		_layerStack.AddLayer(std::move(pLayer));
	}

	void ImgnApp::AddOverlay(unique<Layer> pLayer)
	{
		pLayer->Sleep();
		_layerStack.AddOverlay(std::move(pLayer));
	}
}