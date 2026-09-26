#include "../pch.hpp"
#include "ImgnWindow.h"
#include "ImgnApp.hpp"

namespace Imgn
{
	void ImgnWindow::SetWindowTitle(const std::string& pWindowName)
	{
		_window.SetWindowName(pWindowName.c_str());
	}

	void ImgnWindow::Dream()
	{
		MSG message{};

		// nullptr includes every window on this thread,
		// including the external application's popup host.
		while (PeekMessageW(
			&message, nullptr, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
			{
				ImgnApp::Get().Close();
				return;
			}

			TranslateMessage(&message);
			DispatchMessageW(&message);
		}

		// Preserve Gateware's EVENTS_PROCESSED notification.

		_window.ProcessWindowEvents();
	}

	uint32_t ImgnWindow::GetWidth()
	{
		_window.GetWidth(_width);
		return _width;
	}

	uint32_t ImgnWindow::GetHeight()
	{
		_window.GetHeight(_height);
		return _height;
	}

	float ImgnWindow::GetAspectRatio()
	{
		return static_cast<float>(_width) / static_cast<float>(_height);
	}

	void ImgnWindow::SetEventCallback(const EventCallbackFn& pCallback)
	{
	}
}