#include "../pch.hpp"
#include "ImgnWindow.h"

namespace Imgn
{
	void ImgnWindow::SetWindowTitle(const std::string& pWindowName)
	{
		_window.SetWindowName(pWindowName.c_str());
	}

	void ImgnWindow::Dream()
	{
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