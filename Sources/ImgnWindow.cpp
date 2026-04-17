#include "pch.h"
#include "ImgnWindow.h"

//LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	switch (uMsg)
//	{
//	case WM_CLOSE:
//		DestroyWindow(hwnd);
//		break;
//	case WM_DESTROY:
//		PostQuitMessage(0);
//		return 0;
//	}
//
//	return DefWindowProc(hwnd, uMsg, wParam, lParam);
//}
//
//bool ImgnWindow::ProcessMessages()
//{
//	MSG msg = {};
//
//	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
//	{
//		if (msg.message == WM_QUIT) return false;
//
//		TranslateMessage(&msg);
//		DispatchMessage(&msg);
//	}
//
//	return true;
//}
