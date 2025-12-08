#pragma once
//TODO > Customize completely
// Simple basecode showing how to create a window and attatch a vulkansurface
#define GATEWARE_ENABLE_CORE // All libraries need this
#define GATEWARE_ENABLE_SYSTEM // Graphics libs require system level libraries
#define GATEWARE_ENABLE_INPUT // Graphics libs require system level libraries
#define GATEWARE_DISABLE_GVULKANSURFACE
#define GATEWARE_DISABLE_GOPENGLSURFACE
#define GATEWARE_DISABLE_GDIRECTX12SURFACE
#include "Gateware/Gateware.h"

using namespace GW;
using namespace SYSTEM;
using namespace INPUT;

class Window
{
	static inline std::unique_ptr<Window> _instance;
	static std::mutex _mutex;

	GW::SYSTEM::GWindow _window;
	GW::INPUT::GInput _input;
	GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE _handle;

	Window() = default;

public:
	Window(Window& window) = delete;
	void operator=(const Window&) = delete;
	~Window() = default;

	static Window& GetInstance()
	{
		if (!_instance) _instance.reset(new Window());

		return *_instance;
	}

	bool Init(unsigned width, unsigned height, const std::string& title, GWindowStyle windowStyle = GWindowStyle::WINDOWEDBORDERED)
	{
		//window and handle
		if (-_window.Create(0, 0, width, height, windowStyle)) return false;

		_window.SetWindowName(title.c_str());

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
		_window.GetClientWidth(w);

		return w;
	}

	unsigned GetHeight()
	{
		unsigned h;
		_window.GetClientHeight(h);

		return h;
	}
};

