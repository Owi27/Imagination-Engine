#pragma once
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "../Gateware/Gateware.h"

namespace Imgn
{
	class IMGN_API Input
	{
		static inline unique<Input> _instance;

		void* _windowHandle;
		GInput _input; //todo remove gw
		GBufferedInput _bufferedInput; //todo remove gw
		GEventResponder _responder; //todo remove gw

		Input() /*Constructor*/
		{
			//_windowHandle = pWindow;
			//_instance = Unique<Input>();
			;
			//_bufferedInput.Create(pWindow);

			//_responder.Create([&](const GEvent& e)
			//	{
			//		GBufferedInput::Events event;
			//		GBufferedInput::EVENT_DATA data;

			//		if (+e.Read(event, data))
			//		{
			//			switch (event)
			//			{
			//			case GBufferedInput::Events::Invalid:
			//				break;
			//			case GBufferedInput::Events::KEYPRESSED:
			//				{
			//					KeyPressedEvent kpe(IMGN_KEY(data.data), 1);
			//					EventDispatcher dispatcher(kpe);
			//					dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& e)
			//						{
			//							ImGuiIO& io = ImGui::GetIO();
			//							AddImGuiSpecialKeyEvent(io, e.GetKeyCode(), true);

			//							return false;
			//						});
			//				}
			//				break;
			//			case GBufferedInput::Events::KEYRELEASED:
			//				{
			//					KeyReleasedEvent kpe(IMGN_KEY(data.data));
			//					EventDispatcher dispatcher(kpe);
			//					dispatcher.Dispatch<KeyReleasedEvent>([&](KeyReleasedEvent& e)
			//						{
			//							ImGuiIO& io = ImGui::GetIO();
			//							AddImGuiSpecialKeyEvent(io, e.GetKeyCode(), false);

			//							return false;
			//						});
			//				}
			//				break;
			//			case GBufferedInput::Events::KEYTYPED:
			//				{
			//					KeyTypedEvent kte(data.data);
			//					EventDispatcher dispatcher(kte);
			//					dispatcher.Dispatch<KeyTypedEvent>([&](KeyTypedEvent& e)
			//						{
			//							ImGuiIO& io = ImGui::GetIO();
			//							int keyCode = e.GetKeyCode();

			//							if (keyCode > 0 && keyCode < 0x10000) io.AddInputCharacter(keyCode);

			//							return false;
			//						});
			//				}
			//				break;
			//			case GBufferedInput::Events::BUTTONPRESSED:
			//				{
			//					MouseButtonPressedEvent mme(IMGN_MOUSE(data.data));
			//					EventDispatcher dispatcher(mme);
			//					dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e)
			//						{
			//							ImGuiIO& io = ImGui::GetIO();
			//							io.MouseDown[e.GetMouseButton()] = true;

			//							return false;
			//						});
			//				}
			//				break;
			//			case GBufferedInput::Events::BUTTONRELEASED:
			//				{
			//					MouseButtonReleasedEvent mme(IMGN_MOUSE(data.data));
			//					EventDispatcher dispatcher(mme);
			//					dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e)
			//						{
			//							ImGuiIO& io = ImGui::GetIO();
			//							io.MouseDown[e.GetMouseButton()] = false;


			//							return false;
			//						});
			//				}
			//				break;
			//			case GBufferedInput::Events::MOUSEMOVE:
			//				{
			//					MouseMovedEvent mme(data.x, data.y);
			//					EventDispatcher dispatcher(mme);
			//					dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e)
			//						{
			//							ImGuiIO& io = ImGui::GetIO();
			//							io.MousePos = ImVec2(e.GetX(), e.GetY());

			//							return false;
			//						});

			//				}
			//				break;
			//			case GBufferedInput::Events::MOUSESCROLL:
			//				{
			//					MouseScrolledEvent me(0, 0); //todo
			//					EventDispatcher dispatcher(me);
			//					dispatcher.Dispatch<MouseScrolledEvent>([&](MouseScrolledEvent& e)
			//						{
			//							ImGuiIO& io = ImGui::GetIO();
			//							io.MouseWheelH += e.GetXOffset();
			//							io.MouseWheel += e.GetYOffset();

			//							return false;
			//						});
			//				}
			//				break;
			//			}
			//		}

			//	});
		}

	protected:
		void AttachToWindowImpl(GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE& pWindow);
		bool IsKeyPressedImpl(int pKeyCode);
		bool IsMouseButtonPressedImpl(int pButton);
		std::pair<float, float> GetMousePositionImpl();
		//float GetMouseXImpl(); float GetMouseYImpl();

	public:
		~Input() /*Destructor*/
		{
		}

		inline static Input& Get()
		{
			if (!_instance) _instance.reset(new Input());
			return *_instance;
		}

		/*Copy Constructor*/
		Input(const Input& pOther) = default;

		/*Copy Assignment Operator*/
		Input& operator=(const Input& pOther) = default;

		/*Move Constructor*/
		Input(Input&& pOther) noexcept = default;

		/*Move Assignment Operator*/
		Input& operator=(Input&& pOther) noexcept = default;

		/*Class Functions*/
		static void AttachToWindow(GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE& pWindow);
		static bool IsKeyPressed(int pKeyCode);
		static bool IsMouseButtonPressed(int pButton);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX(); static float GetMouseY();
	};
}