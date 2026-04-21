#include "../pch.hpp"
#include "ImgnWindow.h"

void ImgnWindow::Dream()
{
	_window.ProcessWindowEvents();
}

uint32_t ImgnWindow::GetWidth()
{
	_window.GetClientWidth(_width);
	return _width;
}

uint32_t ImgnWindow::GetHeight()
{
	_window.GetClientWidth(_height);
	return _height;
}

uint32_t ImgnWindow::GetWidth() const
{
	return GetWidth();
}

uint32_t ImgnWindow::GetHeight() const
{
	return GetHeight();
}