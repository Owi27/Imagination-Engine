#pragma once
#include "Gateware.h"

using namespace GW;
using namespace SYSTEM;
using namespace INPUT;

enum ImgnWindowStyle
{
	FULLSCREEN = GWindowStyle::FULLSCREENBORDERLESS,
	FULLSCREEN_WINDOWED = GWindowStyle::FULLSCREENBORDERED,
	WINDOWED = GWindowStyle::WINDOWEDBORDERED,
	WINDOWED_BORDERLESS = GWindowStyle::WINDOWEDBORDERLESS,
	MINIMIZED = GWindowStyle::MINIMIZED,
	LOCKED = GWindowStyle::WINDOWEDLOCKED
};

class ImgnWindow
{
	GW::SYSTEM::GWindow _window;
	GW::INPUT::GInput _input;
	GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE _handle;

public:
	ImgnWindow();
	~ImgnWindow();

	bool Init(unsigned width, unsigned height, const char* title, ImgnWindowStyle windowStyle = WINDOWED)
	{
		//window and handle
		if (-_window.Create(0, 0, width, height, windowStyle)) return false;
		
		_window.SetWindowName(title);

		if (-_window.GetWindowHandle(_handle)) return false;
		
		//input
		if (-_input.Create(_window)) return false;

		return true;
	}

	bool ProcessEvents()
	{
		if (-_window.ProcessWindowEvents()) return false;

		return true;
	}

	bool IsFocused()
	{
		bool isFocused;
		_window.IsFocus(isFocused);

		return isFocused;
	}

	//INPUT

};

ImgnWindow::ImgnWindow()
{
}

ImgnWindow::~ImgnWindow()
{
}