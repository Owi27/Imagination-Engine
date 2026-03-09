#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class ImgnWindow
{
	HINSTANCE _hInstance;
	HWND _hWindow;

public:
	int width = 0, height = 0;

	ImgnWindow(int pX, int pY, int pWidth, int pHeight, LPCWSTR pWindowTitle) /*Constructor*/
	{
		_hInstance = GetModuleHandle(nullptr);

		const wchar_t className[] = L"Imagination Window";
		WNDCLASS windowClass = {};
		windowClass.lpfnWndProc = WindowProc;
		windowClass.hInstance = _hInstance;
		windowClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
		windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		windowClass.lpszClassName = className;

		RegisterClass(&windowClass);

		DWORD style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_MAXIMIZEBOX | WS_OVERLAPPED | WS_THICKFRAME;

		width = pWidth, height = pHeight;

		RECT rect;
		rect.left = pX;
		rect.top = pY;
		rect.right = rect.left + width;
		rect.bottom = rect.top + height;

		AdjustWindowRect(&rect, style, false);

		_hWindow = CreateWindowEx(0, className, pWindowTitle, style, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, _hInstance, nullptr);

		if (_hWindow == nullptr) return;

		ShowWindow(_hWindow, SW_SHOW);
	}

	~ImgnWindow() /*Destructor*/
	{
		const wchar_t className[] = L"Imagination Window";
		UnregisterClass(className, _hInstance);
	}

	HINSTANCE GetInstance() { return _hInstance; }
	HWND GetHandle() { return _hWindow; }
	bool ProcessMessages();

	/*Copy Constructor*/
	ImgnWindow(const ImgnWindow& pOther) = delete;

	/*Copy Assignment Operator*/
	ImgnWindow& operator=(const ImgnWindow& pOther) = delete;

	/*Move Constructor*/
	ImgnWindow(ImgnWindow&& pOther) noexcept = delete;

	/*Move Assignment Operator*/
	ImgnWindow& operator=(ImgnWindow&& pOther) noexcept = delete;
};