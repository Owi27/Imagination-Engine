#pragma once
#include "ImgnCore.hpp"
#include "ImgnLog.h"
#include "Events/Event.h"
#include "ImgnRenderer.h"

struct WindowProperties
{
    std::string title;
    uint32_t width, height;

    WindowProperties(const std::string pTitle = "Imagination Engine", uint32_t pWidth = 1280, uint32_t pHeight = 720)
    {
        title = pTitle;
        width = pWidth;
        height = pHeight;
    }
};

namespace Imgn
{
    class IMGN_API ImgnWindow
    {
        GWindow _window;
        GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE _windowHandle;
        uint32_t _width = 1280, _height = 720;
        GEventResponder _responder;

    public:
        using EventCallbackFn = std::function<void(Event&)>;
        /* Class Functions */
        /*virtual*/ void Dream();
        /*virtual*/ uint32_t GetWidth();
        /*virtual*/ uint32_t GetHeight();
        float GetAspectRatio();
        void AddGEventResponder(GEventResponder& pEventResponder) { _window.Register(pEventResponder); }
        /*virtual*/ void SetEventCallback(const EventCallbackFn& pCallback);
        /*virtual void SetVSync(bool pEnabled);*/
        /*virtual bool IsVSync() const;*/

        //static ImgnWindow* Create(const WindowProperties& pProperties = WindowProperties());

        /* Class Defaults */
        ImgnWindow()
        {
            if (-_window.Create(0, 0, _width, _height, GWindowStyle::FULLSCREENBORDERED)) IMGN_CORE_ERROR("GWindow failed to create | {}:{}", __FILE__, __LINE__);

            _window.GetWindowHandle(_windowHandle);

        }

        /*virtual*/ ~ImgnWindow()
        {

        }

        void SetWindowTitle(const std::string& pWindowName);
        void* GetWindowHandle() { return _windowHandle.window; }
        GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE& GetUniversalWindowHandle() { return _windowHandle; }
        RendererCreateInfo GetRendererCreateInfo(bool pValidation = true, RendererBackend pBackend = RendererBackend::Vulkan)
        {
            return RendererCreateInfo
            {
                .backend = pBackend,
                .windowHandle = _windowHandle.window,
                .width = _width,
                .height = _height,
                .enableValidation = pValidation
            };
        }
    };
}