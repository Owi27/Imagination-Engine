#pragma once

namespace Imgn
{
    class ExternalAppPanel
    {
        bool _appAttached = false;
        PROCESS_INFORMATION _processInfo = {};
        HWND _editorWindow = nullptr, _hostWindow = nullptr, _appWindow = nullptr;

        int _lastX = -1;
        int _lastY = -1;
        uint32_t _lastWidth = 0;
        uint32_t _lastHeight = 0;

        void FocusApp();
        bool FindAppWindow();
        void AttachAppWindow();

    public:
        ExternalAppPanel() /*Constructor*/
        {
        }

        ~ExternalAppPanel() /*Destructor*/
        {
            //
// IMPORTANT:
//
// I'm deliberately NOT calling TerminateProcess here.
//
// Force-killing Blender could cause the user to lose unsaved work.
//

            if (_appWindow && IsWindow(_appWindow)) PostMessageW(_appWindow, WM_CLOSE, 0, 0); // Ask app to close normally.

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
        }

        /*Copy Constructor*/
        ExternalAppPanel(const ExternalAppPanel& pOther) = default;

        /*Copy Assignment Operator*/
        ExternalAppPanel& operator=(const ExternalAppPanel& pOther) = default;

        /*Move Constructor*/
        ExternalAppPanel(ExternalAppPanel&& pOther) noexcept = default;

        /*Move Assignment Operator*/
        ExternalAppPanel& operator=(ExternalAppPanel&& pOther) noexcept = default;

        /*Class Functions*/
        bool Initialize(HWND pEditorWindow);
        bool Launch(const std::filesystem::path& pExe, const std::wstring pArguments = L"");
        void Update(int pScreenX, int pScreenY, uint32_t pWidth, uint32_t pHeight, bool pVisible);
        void Shutdown();
        [[nodiscard]] bool IsRunning() const;
    };

    class AppPanel
    {
        bool _open = true;
        ExternalAppPanel _app;

    public:
        bool Initialize(HWND pEditorWindow, const std::filesystem::path& pExe);
        void Render();
        void Shutdown();

        void Open() { _open = true; }
        void Close() { _open = false; }
        bool IsOpen() const { return _open; }
    };
}