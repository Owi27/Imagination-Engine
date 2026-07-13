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

class IMGN_API ImgnWindow
{
    GWindow _window;
    GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE _windowHandle;
    uint32_t _width = 1280, _height = 720;
    GEventResponder _responder;
    unique<ImgnRenderer> _ctx;

public:
    using EventCallbackFn = std::function<void(Event&)>;
    /* Class Functions */
    /*virtual*/ void Dream();
    /*virtual*/ uint32_t GetWidth();
    /*virtual*/ uint32_t GetHeight();
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

        _ctx = std::make_unique<ImgnRenderer>();
        RendererCreateInfo createInfo
        {
            .windowHandle = _windowHandle.window,
        };

        _ctx->Init(createInfo);
        IMGN_CORE_INFO("Graphics ctx initialized");
        
    }

    /*virtual*/ ~ImgnWindow()
    {

    }
};