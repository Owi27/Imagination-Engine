#include "pch.hpp"
#include "ExternalAppPanel.h"

#include <ImGui/imgui.h>

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
		switch (pMessage)
		{
		case WM_ERASEBKGND:
			// Blender is responsible for drawing this area.
			// Don't let Windows erase the host behind it.
			return 1;

		default:
			return DefWindowProcW(
				pWindow,
				pMessage,
				pWParam,
				pLParam
			);
		}
	}
	BOOL CALLBACK FindProcessWindowCallback(HWND pWindow, LPARAM pParameter)
	{
		auto* pData =
			reinterpret_cast<WindowSearchData*>(
				pParameter
				);

		DWORD processID = 0;

		GetWindowThreadProcessId(
			pWindow,
			&processID
		);

		if (processID != pData->processID)
			return TRUE;

		// We only care about visible application windows.
		if (!IsWindowVisible(pWindow))
			return TRUE;

		// Ignore owned popup/helper windows.
		if (GetWindow(pWindow, GW_OWNER) != nullptr)
			return TRUE;

		RECT rect{};

		if (!GetWindowRect(
			pWindow,
			&rect
		))
		{
			return TRUE;
		}

		const uint32_t width =
			static_cast<uint32_t>(
				rect.right - rect.left
				);

		const uint32_t height =
			static_cast<uint32_t>(
				rect.bottom - rect.top
				);

		if (width == 0 || height == 0)
			return TRUE;

		const uint64_t area =
			static_cast<uint64_t>(width) *
			static_cast<uint64_t>(height);

		// Blender can create more than one HWND during startup.
		// Keep the largest actual application window.
		if (area > pData->largestArea)
		{
			pData->largestArea = area;
			pData->window = pWindow;
		}

		return TRUE;
	}

	void ExternalAppPanel::FocusApp()
	{
		if (!_appWindow || !IsWindow(_appWindow))
			return;

		DWORD appThreadId =
			GetWindowThreadProcessId(
				_appWindow,
				nullptr
			);

		DWORD editorThreadId =
			GetWindowThreadProcessId(
				_editorWindow,
				nullptr
			);

		if (appThreadId == 0 ||
			editorThreadId == 0)
		{
			return;
		}

		// Blender and Imagination have different input queues.
		//
		// Temporarily attach them so Windows allows us to transfer
		// keyboard focus to Blender.
		const bool attached =
			AttachThreadInput(
				editorThreadId,
				appThreadId,
				TRUE
			);

		// Our popup host needs to belong to the active application.
		SetForegroundWindow(
			_hostWindow
		);

		// Give actual keyboard focus to Blender, not just the host.
		SetFocus(
			_appWindow
		);

		if (attached)
		{
			AttachThreadInput(
				editorThreadId,
				appThreadId,
				FALSE
			);
		}
	}

	bool ExternalAppPanel::FindAppWindow()
	{
		if (!_processInfo.dwProcessId)
			return false;

		WindowSearchData searchData
		{
			.processID = _processInfo.dwProcessId
		};

		EnumWindows(
			FindProcessWindowCallback,
			reinterpret_cast<LPARAM>(&searchData)
		);

		if (!searchData.window)
			return false;

		_appWindow = searchData.window;

		IMGN_INFO(
			"Found external HWND: {}",
			reinterpret_cast<uintptr_t>(
				_appWindow
				)
		);

		return true;
	}
	void ExternalAppPanel::AttachAppWindow()
	{
		if (!_appWindow ||
			!_hostWindow)
		{
			return;
		}

		IMGN_INFO(
			"Attaching external application HWND: {}",
			reinterpret_cast<uintptr_t>(
				_appWindow
				)
		);

		LONG_PTR style =
			GetWindowLongPtrW(
				_appWindow,
				GWL_STYLE
			);

		LONG_PTR exStyle =
			GetWindowLongPtrW(
				_appWindow,
				GWL_EXSTYLE
			);

		// Remove normal top-level window decoration.
		style &=
			~WS_POPUP;

		style &=
			~WS_CAPTION;

		style &=
			~WS_THICKFRAME;

		style &=
			~WS_MINIMIZEBOX;

		style &=
			~WS_MAXIMIZEBOX;

		style &=
			~WS_SYSMENU;

		// Blender now lives inside our overlay HWND.
		style |=
			WS_CHILD |
			WS_VISIBLE;

		exStyle &=
			~WS_EX_APPWINDOW;

		SetWindowLongPtrW(
			_appWindow,
			GWL_STYLE,
			style
		);

		SetWindowLongPtrW(
			_appWindow,
			GWL_EXSTYLE,
			exStyle
		);

		SetLastError(
			ERROR_SUCCESS
		);

		HWND oldParent =
			SetParent(
				_appWindow,
				_hostWindow
			);

		const DWORD parentError =
			GetLastError();

		if (!oldParent &&
			parentError != ERROR_SUCCESS)
		{
			IMGN_ERROR(
				"SetParent failed: {}",
				parentError
			);

			return;
		}

		SetWindowPos(
			_appWindow,
			nullptr,

			0,
			0,

			1,
			1,

			SWP_NOZORDER |
			SWP_NOACTIVATE |
			SWP_FRAMECHANGED
		);

		ShowWindow(
			_appWindow,
			SW_SHOW
		);

		_appAttached = true;

		// Force Update() to perform a full placement next frame.
		_lastX = -1;
		_lastY = -1;
		_lastWidth = 0;
		_lastHeight = 0;

		IMGN_INFO(
			"External application attached. "
			"Parent: {} Host: {}",

			reinterpret_cast<uintptr_t>(
				GetParent(_appWindow)
				),

			reinterpret_cast<uintptr_t>(
				_hostWindow
				)
		);
	}
	bool ExternalAppPanel::Initialize(HWND pEditorWindow)
	{
		if (!pEditorWindow)
			return false;

		_editorWindow = pEditorWindow;

		HINSTANCE instance =
			reinterpret_cast<HINSTANCE>(
				GetWindowLongPtrW(
					_editorWindow,
					GWLP_HINSTANCE
				)
				);

		WNDCLASSEXW windowClass
		{
			.cbSize = sizeof(WNDCLASSEXW),

			.style =
				CS_HREDRAW |
				CS_VREDRAW,

			.lpfnWndProc =
				HostWindowProc,

			.hInstance =
				instance,

			.hCursor =
				LoadCursor(
					nullptr,
					IDC_ARROW
				),

			.lpszClassName =
				HOST_WINDOW_CLASS
		};

		if (!RegisterClassExW(
			&windowClass
		))
		{
			const DWORD error =
				GetLastError();

			if (error !=
				ERROR_CLASS_ALREADY_EXISTS)
			{
				return false;
			}
		}

		// IMPORTANT:
		//
		// This is NOT a WS_CHILD anymore.
		//
		// It is an owned, borderless popup window.
		//
		// Because it has its own HWND/redirection surface,
		// the Vulkan swapchain belonging to _editorWindow
		// cannot overwrite it.
		_hostWindow =
			CreateWindowExW(
				WS_EX_TOOLWINDOW,

				HOST_WINDOW_CLASS,

				L"",

				WS_POPUP |
				WS_CLIPCHILDREN,

				0,
				0,
				1,
				1,

				// For a WS_POPUP this establishes the owner,
				// NOT a child-parent relationship.
				_editorWindow,

				nullptr,

				instance,

				nullptr
			);

		if (!_hostWindow)
			return false;

		return true;
	}
	bool ExternalAppPanel::Launch(const std::filesystem::path& pExe, const std::wstring pArguments)
	{
		if (!_hostWindow)
			return false;

		if (!std::filesystem::exists(pExe))
			return false;

		if (IsRunning())
			return true;

		std::wstring commandLine =
			L"\"" +
			pExe.wstring() +
			L"\"";

		if (!pArguments.empty())
		{
			commandLine += L" ";
			commandLine += pArguments;
		}

		std::vector<wchar_t> commandBuffer(
			commandLine.begin(),
			commandLine.end()
		);

		commandBuffer.push_back(
			L'\0'
		);

		STARTUPINFOW startupInfo{};
		startupInfo.cb =
			sizeof(STARTUPINFOW);

		ZeroMemory(
			&_processInfo,
			sizeof(PROCESS_INFORMATION)
		);

		const BOOL success =
			CreateProcessW(
				pExe.c_str(),

				commandBuffer.data(),

				nullptr,
				nullptr,

				FALSE,

				CREATE_NEW_PROCESS_GROUP,

				nullptr,

				pExe.parent_path().c_str(),

				&startupInfo,

				&_processInfo
			);

		if (!success)
		{
			IMGN_ERROR(
				"CreateProcessW failed: {}",
				GetLastError()
			);

			return false;
		}

		WaitForInputIdle(
			_processInfo.hProcess,
			5000
		);

		return true;
	}
	void ExternalAppPanel::Update(int pScreenX, int pScreenY, uint32_t pWidth, uint32_t pHeight, bool pVisible)
	{
		if (!_hostWindow)
			return;

		// Blender may take a moment to create its main HWND.
		if (!_appAttached && IsRunning())
		{
			if (FindAppWindow())
				AttachAppWindow();
		}

		// --------------------------------------------------------
		// DETERMINE WHETHER THIS PANEL SHOULD BE SHOWN
		// --------------------------------------------------------

		const bool editorVisible =
			IsWindowVisible(_editorWindow);

		const bool editorMinimized =
			IsIconic(_editorWindow);

		const bool shouldShow =
			pVisible &&
			pWidth > 0 &&
			pHeight > 0 &&
			editorVisible &&
			!editorMinimized;

		if (!shouldShow)
		{
			if (IsWindowVisible(_hostWindow))
				ShowWindow(_hostWindow, SW_HIDE);

			return;
		}

		// This is a WS_POPUP now.
		//
		// ImGui::GetCursorScreenPos() already gave us SCREEN coordinates,
		// so DO NOT ScreenToClient() these.
		const int x = pScreenX;
		const int y = pScreenY;

		const bool rectChanged =
			x != _lastX ||
			y != _lastY ||
			pWidth != _lastWidth ||
			pHeight != _lastHeight;

		// --------------------------------------------------------
		// ONLY MOVE/RESIZE WHEN THE IMGUI RECT ACTUALLY CHANGED
		// --------------------------------------------------------

		if (rectChanged)
		{
			_lastX = x;
			_lastY = y;
			_lastWidth = pWidth;
			_lastHeight = pHeight;

			SetWindowPos(
				_hostWindow,
				HWND_TOP,
				x,
				y,
				static_cast<int>(pWidth),
				static_cast<int>(pHeight),
				SWP_NOACTIVATE |
				SWP_SHOWWINDOW
			);

			if (_appAttached &&
				_appWindow &&
				IsWindow(_appWindow))
			{
				SetWindowPos(
					_appWindow,
					nullptr,
					0,
					0,
					static_cast<int>(pWidth),
					static_cast<int>(pHeight),
					SWP_NOZORDER |
					SWP_NOACTIVATE |
					SWP_SHOWWINDOW
				);
			}
		}
		else
		{
			// IMPORTANT:
			//
			// Don't resize anything.
			//
			// But reassert that our popup belongs on top of its owner.
			// Switching back from SceneView can change the native
			// activation/Z-order even though the rectangle didn't change.
			SetWindowPos(
				_hostWindow,
				HWND_TOP,
				0,
				0,
				0,
				0,
				SWP_NOMOVE |
				SWP_NOSIZE |
				SWP_NOACTIVATE |
				SWP_SHOWWINDOW
			);
		}
	}
	void ExternalAppPanel::Shutdown()
	{
		if (_hostWindow)
		{
			ShowWindow(
				_hostWindow,
				SW_HIDE
			);

		}

		if (_appWindow &&
			IsWindow(_appWindow))
		{
			PostMessageW(
				_appWindow,
				WM_CLOSE,
				0,
				0
			);
		}

		_appWindow = nullptr;
		_appAttached = false;

		if (_hostWindow)
		{
			DestroyWindow(
				_hostWindow
			);

			_hostWindow = nullptr;
		}

		if (_processInfo.hThread)
		{
			CloseHandle(
				_processInfo.hThread
			);

			_processInfo.hThread =
				nullptr;
		}

		if (_processInfo.hProcess)
		{
			CloseHandle(
				_processInfo.hProcess
			);

			_processInfo.hProcess =
				nullptr;
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
		if (!_app.Initialize(
			pEditorWindow
		))
		{
			return false;
		}

		return _app.Launch(
			pExe
		);
	}
	void AppPanel::Render()
	{
		if (!_open)
		{
			_app.Update(
				0,
				0,
				0,
				0,
				false
			);

			return;
		}

		ImGui::PushStyleVar(
			ImGuiStyleVar_WindowPadding,
			ImVec2(
				0.0f,
				0.0f
			)
		);

		const bool visible =
			ImGui::Begin(
				"ExternalApp",

				&_open,

				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoScrollWithMouse
			);

		if (visible)
		{
			const ImVec2 position =
				ImGui::GetCursorScreenPos();

			const ImVec2 size =
				ImGui::GetContentRegionAvail();

			if (size.x > 0.0f &&
				size.y > 0.0f)
			{
				_app.Update(
					static_cast<int>(
						position.x
						),

					static_cast<int>(
						position.y
						),

					static_cast<uint32_t>(
						size.x
						),

					static_cast<uint32_t>(
						size.y
						),

					true
				);

				// Reserve the region inside ImGui.
				ImGui::Dummy(
					size
				);
			}
		}
		else
		{
			// An inactive docked tab returns false from Begin().
			_app.Update(
				0,
				0,
				0,
				0,
				false
			);
		}

		ImGui::End();

		ImGui::PopStyleVar();
	}
	void AppPanel::Shutdown()
	{
		_app.Shutdown();
	}
}