#pragma once
// Simple basecode showing how to create a window and attatch a vulkansurface
#define GATEWARE_ENABLE_CORE // All libraries need this
#define GATEWARE_ENABLE_SYSTEM // Graphics libs require system level libraries
#define GATEWARE_ENABLE_INPUT // Graphics libs require system level libraries
#define GATEWARE_DISABLE_GVULKANSURFACE
#define GATEWARE_DISABLE_GOPENGLSURFACE
#define GATEWARE_DISABLE_GDIRECTX12SURFACE
#include "Gateware.h"

using namespace GW;
using namespace SYSTEM;
using namespace INPUT;
using ImgnWindowStyl = GWindowStyle;

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
	static inline std::unique_ptr<ImgnWindow> _instance;
	static std::mutex _mutex;

	GW::SYSTEM::GWindow _window;
	GW::INPUT::GInput _input;
	GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE _handle;

	ImgnWindow() = default;

public:
	ImgnWindow(ImgnWindow& window) = delete;
	void operator=(const ImgnWindow&) = delete;
	~ImgnWindow() = default;

	static ImgnWindow& GetInstance()
	{
		if (!_instance) _instance.reset(new ImgnWindow());

		return *_instance;
	}

	bool Init(unsigned width, unsigned height, const char* title, ImgnWindowStyle windowStyle = WINDOWED)
	{
		//window and handle
		if (-_window.Create(0, 0, width, height, ImgnWindowStyl::WINDOWEDBORDERED)) return false;
		
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

	void* GetWindowHandle()
	{
		return _handle.window;
	}

	void* GetDisplayHandle()
	{
		return _handle.display;
	}

	unsigned GetWidth()
	{
		unsigned w;
		_window.GetWidth(w);

		return w;
	}

	unsigned GetHeight()
	{
		unsigned h;
		_window.GetHeight(h);

		return h;
	}

	//INPUT

};