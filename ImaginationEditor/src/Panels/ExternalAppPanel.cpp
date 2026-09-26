#include "pch.hpp"
#include "ExternalAppPanel.h"

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>

namespace Imgn
{
	constexpr wchar_t HOST_WINDOW_CLASS[] = L"ImaginationExternalApplicationHost";

	struct WindowSearchData
	{
		DWORD processID = 0;
		HWND window = nullptr;
		uint64_t largestArea = 0;
	};

	LRESULT CALLBACK HostWindowProc(HWND pWindow, UINT pMessage, WPARAM pWParam, LPARAM pLParam)
	{
		constexpr UINT WM_FOCUS_EXTERNAL_APP = WM_APP + 1;

		if (pMessage == WM_NCCREATE)
		{
			const auto* create = reinterpret_cast<const CREATESTRUCTW*>(pLParam);

			SetWindowLongPtrW(pWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
			return TRUE;
		}

		auto* panel = reinterpret_cast<ExternalAppPanel*>(GetWindowLongPtrW(pWindow, GWLP_USERDATA));

		switch (pMessage)
		{
		case WM_MOUSEACTIVATE:
			//defer focus until Windows finishes activation.
			PostMessageW(pWindow, WM_FOCUS_EXTERNAL_APP, 0, 0);
			return MA_ACTIVATE; //also deliver the original click.
		case WM_PARENTNOTIFY:
			switch (LOWORD(pWParam))
			{
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_XBUTTONDOWN:
				PostMessageW(pWindow, WM_FOCUS_EXTERNAL_APP, 0, 0);
				break;
			}
			break;
		case WM_SETFOCUS:
			PostMessageW(pWindow, WM_FOCUS_EXTERNAL_APP, 0, 0);
			return 0;
		case WM_FOCUS_EXTERNAL_APP:
			if (panel) panel->FocusApp();
			return 0;
		case WM_ERASEBKGND:
			return 1;
		case WM_NCDESTROY:
			SetWindowLongPtrW(pWindow, GWLP_USERDATA, 0);
			break;
		}

		return DefWindowProcW(pWindow, pMessage, pWParam, pLParam);
	}
	BOOL CALLBACK FindProcessWindowCallback(HWND pWindow, LPARAM pParameter)
	{
		auto* pData = reinterpret_cast<WindowSearchData*>(pParameter);
		DWORD processID = 0;

		GetWindowThreadProcessId(pWindow, &processID);

		if (processID != pData->processID) return TRUE;

		//only care about visible application windows.
		if (!IsWindowVisible(pWindow)) return TRUE;

		//ignore owned popup/helper windows.
		if (GetWindow(pWindow, GW_OWNER) != nullptr) return TRUE;

		RECT rect{};

		if (!GetWindowRect(pWindow, &rect)) return TRUE;

		const uint32_t width = static_cast<uint32_t>(rect.right - rect.left);
		const uint32_t height = static_cast<uint32_t>(rect.bottom - rect.top);

		if (width == 0 || height == 0) return TRUE;

		const uint64_t area = static_cast<uint64_t>(width) * static_cast<uint64_t>(height);

		//external can create more than one HWND during startup.
		//keep the largest actual application window.
		if (area > pData->largestArea)
		{
			pData->largestArea = area;
			pData->window = pWindow;
		}

		return TRUE;
	}

	void ExternalAppPanel::FocusApp()
	{
		if (!_appAttached || !IsWindow(_appWindow) || !IsWindowVisible(_hostWindow) || !IsWindowEnabled(_appWindow)) return;

		//ignore delayed requests after the user switches elsewhere.
		if (GetForegroundWindow() != _hostWindow) return;

		const DWORD appThread = GetWindowThreadProcessId(_appWindow, nullptr);
		const DWORD ourThread = GetCurrentThreadId();

		if (appThread == 0) return;

		//preserve focus if external already has it.
		GUITHREADINFO info
		{
			.cbSize = sizeof(info)
		};

		if (GetGUIThreadInfo(appThread, &info) && (info.hwndFocus == _appWindow || IsChild(_appWindow, info.hwndFocus))) return;

		const bool needAttach = appThread != ourThread;

		if (needAttach && !AttachThreadInput(ourThread, appThread, TRUE))
		{
			IMGN_ERROR("AttachThreadInput failed: {}", GetLastError());
			return;
		}

		SetFocus(_appWindow);

		const HWND focused = GetFocus();
		const bool success = focused == _appWindow || IsChild(_appWindow, focused);

		if (needAttach) if (!AttachThreadInput(ourThread, appThread, FALSE)) IMGN_ERROR("DetachThreadInput failed: {}", GetLastError());
		if (!success) IMGN_ERROR("External App did not receive keyboard focus.");
	}

	bool ExternalAppPanel::FindAppWindow()
	{
		if (!_processInfo.dwProcessId) return false;

		WindowSearchData searchData
		{
			.processID = _processInfo.dwProcessId
		};

		EnumWindows(FindProcessWindowCallback, reinterpret_cast<LPARAM>(&searchData));

		if (!searchData.window) return false;

		_appWindow = searchData.window;

		IMGN_INFO("Found external HWND: {}", reinterpret_cast<uintptr_t>(_appWindow));

		return true;
	}
	void ExternalAppPanel::AttachAppWindow()
	{
		if (!_appWindow || !_hostWindow) return;

		IMGN_INFO("Attaching external application HWND: {}", reinterpret_cast<uintptr_t>(_appWindow));

		LONG_PTR style = GetWindowLongPtrW(_appWindow, GWL_STYLE);
		LONG_PTR exStyle = GetWindowLongPtrW(_appWindow, GWL_EXSTYLE);

		//remove normal top-level window decoration.
		style &= ~WS_POPUP;
		style &= ~WS_CAPTION;
		style &= ~WS_THICKFRAME;
		style &= ~WS_MINIMIZEBOX;
		style &= ~WS_MAXIMIZEBOX;
		style &= ~WS_SYSMENU;

		//external app now lives inside our overlay HWND.
		style |= WS_CHILD | WS_VISIBLE;
		exStyle &= ~(WS_EX_APPWINDOW | WS_EX_NOPARENTNOTIFY);

		SetWindowLongPtrW(_appWindow, GWL_STYLE, style);
		SetWindowLongPtrW(_appWindow, GWL_EXSTYLE, exStyle);
		SetLastError(ERROR_SUCCESS);

		HWND oldParent = SetParent(_appWindow, _hostWindow);
		const DWORD parentError = GetLastError();

		if (!oldParent && parentError != ERROR_SUCCESS)
		{
			IMGN_ERROR("SetParent failed: {}", parentError);
			return;
		}

		SetWindowPos(_appWindow, nullptr, 0, 0, 1, 1, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		ShowWindow(_appWindow, SW_SHOW);

		_appAttached = true;

		//force Update() to perform a full placement next frame.
		_lastX = -1;
		_lastY = -1;
		_lastWidth = 0;
		_lastHeight = 0;

		IMGN_INFO( "External application attached. Parent: {} Host: {}", reinterpret_cast<uintptr_t>(GetParent(_appWindow)), reinterpret_cast<uintptr_t>(_hostWindow));
	}
	bool ExternalAppPanel::Initialize(HWND pEditorWindow)
	{
		if (!pEditorWindow) return false;

		_editorWindow = pEditorWindow;

		HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(_editorWindow, GWLP_HINSTANCE));

		WNDCLASSEXW windowClass
		{
			.cbSize = sizeof(WNDCLASSEXW),
			.style = CS_HREDRAW | CS_VREDRAW,
			.lpfnWndProc = HostWindowProc,
			.hInstance = instance,
			.hCursor = LoadCursor(nullptr, IDC_ARROW),
			.lpszClassName = HOST_WINDOW_CLASS
		};

		if (!RegisterClassExW(&windowClass))
		{
			const DWORD error = GetLastError();

			if (error != ERROR_CLASS_ALREADY_EXISTS) return false;
		}

		_hostWindow = CreateWindowExW(WS_EX_TOOLWINDOW, HOST_WINDOW_CLASS, L"", WS_POPUP | WS_CLIPCHILDREN, 0, 0, 1, 1, _editorWindow, nullptr, instance, nullptr);

		if (!_hostWindow) return false;

		return true;

	}
	bool ExternalAppPanel::Launch(const std::filesystem::path& pExe, const std::wstring pArguments)
	{
		if (!_hostWindow || !std::filesystem::exists(pExe)) return false;
		if (IsRunning()) return true;

		std::wstring commandLine = L"\"" + pExe.wstring() + L"\"";

		if (!pArguments.empty())
		{
			commandLine += L" ";
			commandLine += pArguments;
		}

		std::vector<wchar_t> commandBuffer(commandLine.begin(), commandLine.end());
		commandBuffer.push_back(L'\0');

		STARTUPINFOW startupInfo
		{
			.cb = sizeof(STARTUPINFOW)
		};

		ZeroMemory(&_processInfo, sizeof(PROCESS_INFORMATION));

		const BOOL success = CreateProcessW(pExe.c_str(), commandBuffer.data(), nullptr, nullptr, FALSE, CREATE_NEW_PROCESS_GROUP, nullptr, pExe.parent_path().c_str(), &startupInfo, &_processInfo);

		if (!success)
		{
			IMGN_ERROR("CreateProcessW failed: {}", GetLastError());
			return false;
		}

		WaitForInputIdle(_processInfo.hProcess, 5000);

		return true;
	}
	void ExternalAppPanel::Update(int pScreenX, int pScreenY, uint32_t pWidth, uint32_t pHeight, bool pVisible)
	{
		if (!_hostWindow || !IsWindow(_hostWindow)) return;
		if (_appAttached && !IsWindow(_appWindow))
		{
			_appAttached = false;
			_appWindow = nullptr;
		}

		if (!_appAttached && IsRunning() && FindAppWindow()) AttachAppWindow();

		const bool show = pVisible && pWidth > 0 && pHeight > 0 && _appAttached && IsWindow(_appWindow) && IsWindowVisible(_editorWindow) && !IsIconic(_editorWindow);

		if (!show)
		{
			if (IsWindowVisible(_hostWindow)) ShowWindow(_hostWindow, SW_HIDE);
			return;
		}

		const bool changed = pScreenX != _lastX || pScreenY != _lastY || pWidth != _lastWidth || pHeight != _lastHeight;
		const bool hidden = !IsWindowVisible(_hostWindow);

		if (changed) SetWindowPos(_appWindow, nullptr, 0, 0, static_cast<int>(pWidth), static_cast<int>(pHeight), SWP_NOZORDER | SWP_NOACTIVATE);
		if (changed || hidden)
		{
			const UINT flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW;

			if (SetWindowPos(_hostWindow, nullptr, pScreenX, pScreenY, static_cast<int>(pWidth), static_cast<int>(pHeight), flags))
			{
				_lastX = pScreenX;
				_lastY = pScreenY;
				_lastWidth = pWidth;
				_lastHeight = pHeight;
			}
		}
	}
	void ExternalAppPanel::Shutdown()
	{
		if (_hostWindow)
		{
			ShowWindow(_hostWindow, SW_HIDE);
		}

		if (_appWindow && IsWindow(_appWindow))
		{
			PostMessageW(_appWindow, WM_CLOSE, 0, 0);
		}

		_appWindow = nullptr;
		_appAttached = false;

		if (_hostWindow)
		{
			DestroyWindow(_hostWindow);

			_hostWindow = nullptr;
		}

		if (_processInfo.hThread)
		{
			CloseHandle(_processInfo.hThread);

			_processInfo.hThread = nullptr;
		}

		if (_processInfo.hProcess)
		{
			CloseHandle(_processInfo.hProcess);

			_processInfo.hProcess = nullptr;
		}

		_processInfo.dwProcessId = 0;
		_processInfo.dwThreadId = 0;

		_editorWindow = nullptr;

		_lastX = -1;
		_lastY = -1;
		_lastWidth = 0;
		_lastHeight = 0;
	}
	bool ExternalAppPanel::IsRunning() const
	{
		if (!_processInfo.hProcess) return false;

		return WaitForSingleObject(_processInfo.hProcess, 0) == WAIT_TIMEOUT;

		//const DWORD result = WaitForSingleObject(_processInfo.hProcess, 0);

		//return result == WAIT_TIMEOUT;
	}
	bool AppPanel::Initialize(HWND pEditorWindow, const std::filesystem::path& pExe)
	{
		if (!_app.Initialize(pEditorWindow)) return false;

		return _app.Launch(pExe);
	}
	void AppPanel::Render()
	{
		if (!_open)
		{
			_app.Update(0, 0, 0, 0, false);
			return;
		}

		// Leave space around Blender for ImGui resize interactions.
		ImGui::SetNextWindowSize(ImVec2(960.f, 640.f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(ImVec2(320.f, 240.f), ImVec2(FLT_MAX, FLT_MAX));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
		const bool visible = ImGui::Begin("ExternalApp", &_open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::PopStyleVar();

		ImGuiContext& context = *ImGui::GetCurrentContext();
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		const ImVec2 position = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImGui::GetContentRegionAvail();

		ImRect area(position, ImVec2(position.x + size.x, position.y + size.y));
		area.ClipWith(window->InnerClipRect);

		const ImVec2 viewportEnd(window->Viewport->Pos.x + window->Viewport->Size.x, window->Viewport->Pos.y + window->Viewport->Size.y);

		area.ClipWith(ImRect(window->Viewport->Pos, viewportEnd));

		const bool uiDragging = context.MovingWindow != nullptr || context.DragDropActive;
		const bool popupOpen = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
		const bool selectedTab = !window->DockIsActive || window->DockTabIsVisible;
		bool show = visible && _open && !window->Hidden && selectedTab && area.GetWidth() >= 1.f && area.GetHeight() >= 1.f && !uiDragging && !popupOpen;

		if (show)
			for (ImGuiWindow* other : context.Windows)
			{
				if (!other->Active || other->Hidden || (other->IsFallbackWindow && !other->WriteAccessed) || other->RootWindowDockTree == window->RootWindowDockTree || !ImGui::IsWindowAbove(other, window) || !other->OuterRectClipped.Overlaps(area)) continue;

				// Ignore empty, noninteractive overlays such as ImGuizmo's "gizmo".
				const bool noInputs = (other->Flags & ImGuiWindowFlags_NoInputs) == ImGuiWindowFlags_NoInputs;
				const bool hasDrawing = std::any_of(other->DrawList->CmdBuffer.begin(), other->DrawList->CmdBuffer.end(), [](const ImDrawCmd& cmd) { return cmd.ElemCount != 0 || cmd.UserCallback != nullptr; });

				if (noInputs && !hasDrawing) continue;

				show = false;
				break;
			}

		POINT topLeft = { static_cast<LONG>(std::ceil(area.Min.x)), static_cast<LONG>(std::ceil(area.Min.y)) };
		POINT bottomRight = { static_cast<LONG>(std::floor(area.Max.x)), static_cast<LONG>(std::floor(area.Max.y)) };

		// Single-viewport ImGui coordinates are relative to the editor client area.
		if (show && !(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable))
		{
			HWND editor = static_cast<HWND>(ImGui::GetMainViewport()->PlatformHandleRaw);
			show = editor && ClientToScreen(editor, &topLeft) && ClientToScreen(editor, &bottomRight);
		}

		if (show && bottomRight.x > topLeft.x && bottomRight.y > topLeft.y) _app.Update(topLeft.x, topLeft.y, static_cast<uint32_t>(bottomRight.x - topLeft.x), static_cast<uint32_t>(bottomRight.y - topLeft.y), true);
		else _app.Update(0, 0, 0, 0, false);

		if (visible && size.x > 0.f && size.y > 0.f) ImGui::Dummy(size);

		ImGui::End();
	}
	void AppPanel::Shutdown()
	{
		_app.Shutdown();
	}
}