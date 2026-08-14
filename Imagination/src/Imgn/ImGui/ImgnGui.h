#pragma once
#include "Imgn/ImgnLayer.h"
#include "Imgn/ImgnWindow.h"
#include "Imgn/ImgnRenderer.h"

namespace Imgn
{
    class IMGN_API ImGuiLayer : public Layer
    {
        ImgnWindow* _window;
        ImgnRenderer* _renderer;

        GEventResponder _responder;

        static int ToImGuiMouseButton(int pButton);
        static void AddImGuiSpecialInputEvent(ImGuiIO& pIO, int pKeyCode, bool pPressed);
        static int ImGui_ImplWin32_CreateVkSurface(ImGuiViewport* pViewport, ImU64 pVkInstance, const void* pVkAllocator, ImU64* pOutSurface);

    public:
        ImGuiLayer() : Layer("ImGuiLayer") /*Constructor*/
        {
        }

        ImGuiLayer(ImgnWindow* pWindow, ImgnRenderer* pRenderer) : Layer("ImGuiLayer")
        {
            _renderer = pRenderer;
            _window = pWindow;
        }

        ~ImGuiLayer() /*Destructor*/
        {
        }

        /*Copy Constructor*/
        ImGuiLayer(const ImGuiLayer& pOther) = default;

        /*Copy Assignment Operator*/
        ImGuiLayer& operator=(const ImGuiLayer& pOther) = default;

        /*Move Constructor*/
        ImGuiLayer(ImGuiLayer&& pOther) noexcept = default;

        /*Move Assignment Operator*/
        ImGuiLayer& operator=(ImGuiLayer&& pOther) noexcept = default;

        /*Class Functions*/
        void Sleep();
        void WakeUp();
        void Dream(Time pTime);
        void OnEvent(Event& pEvent);

    };
}