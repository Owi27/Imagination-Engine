#include "pch.hpp"
#include "ImgnInput.h"
#include "ImgnApp.hpp"
#include "../Gateware/Gateware.h"

namespace Imgn
{
	void Input::AttachToWindowImpl(GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE& pWindow)
	{
		_input.Create(pWindow);
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

	float Input::GetMouseX()
	{
		return std::get<0>(GetMousePosition());
	}

	float Input::GetMouseY()
	{
		return std::get<1>(GetMousePosition());
	}

}