#include "pch.hpp"
#include "ImgnInput.h"
#include "ImgnApp.hpp"
#include "../Gateware/Gateware.h"

namespace Imgn
{
	void Input::OnGatewareEvent(const GW::GEvent& pEvent)
	{
		GBufferedInput::Events event;
		GBufferedInput::EVENT_DATA data;

		if (-pEvent.Read(event, data)) return;

		switch (event)
		{
		case GW::INPUT::GBufferedInput::Events::KEYPRESSED:
			{
				KeyPressedEvent event(data.data, 0);

				if (_eventCallback) _eventCallback(event);

				break;
			}

		case GW::INPUT::GBufferedInput::Events::KEYRELEASED:
			{
				KeyReleasedEvent event(data.data);

				if (_eventCallback) _eventCallback(event);

				break;
			}

		case GW::INPUT::GBufferedInput::Events::BUTTONPRESSED:
			{
				MouseButtonPressedEvent event(data.data);

				if (_eventCallback) _eventCallback(event);

				break;
			}

		case GW::INPUT::GBufferedInput::Events::BUTTONRELEASED:
			{
				MouseButtonReleasedEvent event(data.data);

				if (_eventCallback) _eventCallback(event);

				break;
			}

		case GW::INPUT::GBufferedInput::Events::MOUSEMOVE:
			{
				MouseMovedEvent event(static_cast<float>(data.x), static_cast<float>(data.y));

				if (_eventCallback) _eventCallback(event);

				break;
			}

		case GW::INPUT::GBufferedInput::Events::MOUSESCROLL:
			{
				float y = 0.0f;

				if (data.data == G_MOUSE_SCROLL_UP) y = 1.0f;
				else if (data.data == G_MOUSE_SCROLL_DOWN) y = -1.0f;

				MouseScrolledEvent event(0.0f, y);

				if (_eventCallback) _eventCallback(event);

				break;
			}

		default:
			break;
		}
	}

	void Input::AttachToWindowImpl(GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE& pWindow)
	{
		_input.Create(pWindow);
		_bufferedInput.Create(pWindow);
		_responder.Create([this](const GEvent& e) { OnGatewareEvent(e); });
		_bufferedInput.Register(_responder);
	}

	bool Input::IsKeyPressedImpl(int pKeyCode)
	{
		float state = 0.f;
		_input.GetState(pKeyCode, state);

		return state > 0.f;
	}

	bool Input::IsMouseButtonPressedImpl(int pButton)
	{
		float state = 0.f;
		_input.GetState(pButton, state);

		return state > 0.f;
	}

	std::pair<float, float> Input::GetMousePositionImpl()
	{
		float x; float y;
		_input.GetMousePosition(x, y);

		return { x, y };
	}

	std::pair<float, float> Input::GetMouseDeltaImpl()
	{
		float x = 0.f; float y = 0.f;
		GW::GReturn result = _input.GetMouseDelta(x, y);

		if (!G_PASS(result) || result == GW::GReturn::REDUNDANT) return { 0.f, 0.f };

		return { x, y };
	}

	void Input::AttachToWindow(GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE& pWindow)
	{
		return _instance->AttachToWindowImpl(pWindow);
	}

	bool Input::IsKeyPressed(int pKeyCode)
	{
		return _instance->IsKeyPressedImpl(pKeyCode);
	}

	bool Input::IsMouseButtonPressed(int pButton)
	{
		return _instance->IsMouseButtonPressedImpl(pButton);
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		return _instance->GetMousePositionImpl();
	}

	std::pair<float, float> Input::GetMouseDelta()
	{
		return _instance->GetMouseDeltaImpl();
	}

	float Input::GetMouseX()
	{
		return std::get<0>(GetMousePosition());
	}

	float Input::GetMouseY()
	{
		return std::get<1>(GetMousePosition());
	}

	float Input::GetMouseXDelta()
	{
		return std::get<0>(GetMouseDelta());
	}

	float Input::GetMouseYDelta()
	{
		return std::get<1>(GetMouseDelta());
	}

	void Input::SetEventCallback(EventCallbackFn pCallback)
	{
		_instance->_eventCallback = std::move(pCallback);
	}

}