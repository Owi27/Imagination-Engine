#include "pch.hpp"
#include "ImgnGui.h"
#include "Imgn/ImgnApp.hpp"

#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_vulkan.h"

namespace Imgn
{
	void ImGuiLayer::AddImGuiSpecialInputEvent(ImGuiIO& pIO, int pKeyCode, bool pPressed)
	{
		switch (pKeyCode)
		{
		case IMGN_KEY_TAB:             pIO.AddKeyEvent(ImGuiKey_Tab, pPressed); break;
		case IMGN_KEY_LEFT:            pIO.AddKeyEvent(ImGuiKey_LeftArrow, pPressed); break;
		case IMGN_KEY_RIGHT:           pIO.AddKeyEvent(ImGuiKey_RightArrow, pPressed); break;
		case IMGN_KEY_UP:              pIO.AddKeyEvent(ImGuiKey_UpArrow, pPressed); break;
		case IMGN_KEY_DOWN:            pIO.AddKeyEvent(ImGuiKey_DownArrow, pPressed); break;
		case IMGN_KEY_PAGE_UP:         pIO.AddKeyEvent(ImGuiKey_PageUp, pPressed); break;
		case IMGN_KEY_PAGE_DOWN:       pIO.AddKeyEvent(ImGuiKey_PageDown, pPressed); break;
		case IMGN_KEY_HOME:            pIO.AddKeyEvent(ImGuiKey_Home, pPressed); break;
		case IMGN_KEY_END:             pIO.AddKeyEvent(ImGuiKey_End, pPressed); break;
		case IMGN_KEY_INSERT:          pIO.AddKeyEvent(ImGuiKey_Insert, pPressed); break;
		case IMGN_KEY_DELETE:          pIO.AddKeyEvent(ImGuiKey_Delete, pPressed); break;
		case IMGN_KEY_BACKSPACE:       pIO.AddKeyEvent(ImGuiKey_Backspace, pPressed); break;
		case IMGN_KEY_SPACE:           pIO.AddKeyEvent(ImGuiKey_Space, pPressed); break;
		case IMGN_KEY_ENTER:           pIO.AddKeyEvent(ImGuiKey_Enter, pPressed); break;
		case IMGN_KEY_ESCAPE:          pIO.AddKeyEvent(ImGuiKey_Escape, pPressed); break;

		case IMGN_KEY_LEFT_CONTROL:    pIO.AddKeyEvent(ImGuiKey_LeftCtrl, pPressed); break;
		case IMGN_KEY_LEFT_SHIFT:      pIO.AddKeyEvent(ImGuiKey_LeftShift, pPressed); break;
		case IMGN_KEY_LEFT_ALT:        pIO.AddKeyEvent(ImGuiKey_LeftAlt, pPressed); break;
		case IMGN_KEY_LEFT_SUPER:      pIO.AddKeyEvent(ImGuiKey_LeftSuper, pPressed); break;
		case IMGN_KEY_RIGHT_CONTROL:   pIO.AddKeyEvent(ImGuiKey_RightCtrl, pPressed); break;
		case IMGN_KEY_RIGHT_SHIFT:     pIO.AddKeyEvent(ImGuiKey_RightShift, pPressed); break;
		case IMGN_KEY_RIGHT_ALT:       pIO.AddKeyEvent(ImGuiKey_RightAlt, pPressed); break;
			//case IMGN_KEY_RIGHT_SUPER:     pIO.AddKeyEvent(ImGuiKey_RightSuper, pPressed); break;
			//case IMGN_KEY_MENU:            pIO.AddKeyEvent(ImGuiKey_Menu, pPressed); break;

		case IMGN_KEY_0:               pIO.AddKeyEvent(ImGuiKey_0, pPressed); break;
		case IMGN_KEY_1:               pIO.AddKeyEvent(ImGuiKey_1, pPressed); break;
		case IMGN_KEY_2:               pIO.AddKeyEvent(ImGuiKey_2, pPressed); break;
		case IMGN_KEY_3:               pIO.AddKeyEvent(ImGuiKey_3, pPressed); break;
		case IMGN_KEY_4:               pIO.AddKeyEvent(ImGuiKey_4, pPressed); break;
		case IMGN_KEY_5:               pIO.AddKeyEvent(ImGuiKey_5, pPressed); break;
		case IMGN_KEY_6:               pIO.AddKeyEvent(ImGuiKey_6, pPressed); break;
		case IMGN_KEY_7:               pIO.AddKeyEvent(ImGuiKey_7, pPressed); break;
		case IMGN_KEY_8:               pIO.AddKeyEvent(ImGuiKey_8, pPressed); break;
		case IMGN_KEY_9:               pIO.AddKeyEvent(ImGuiKey_9, pPressed); break;

		case IMGN_KEY_A:               pIO.AddKeyEvent(ImGuiKey_A, pPressed); break;
		case IMGN_KEY_B:               pIO.AddKeyEvent(ImGuiKey_B, pPressed); break;
		case IMGN_KEY_C:               pIO.AddKeyEvent(ImGuiKey_C, pPressed); break;
		case IMGN_KEY_D:               pIO.AddKeyEvent(ImGuiKey_D, pPressed); break;
		case IMGN_KEY_E:               pIO.AddKeyEvent(ImGuiKey_E, pPressed); break;
		case IMGN_KEY_F:               pIO.AddKeyEvent(ImGuiKey_F, pPressed); break;
		case IMGN_KEY_G:               pIO.AddKeyEvent(ImGuiKey_G, pPressed); break;
		case IMGN_KEY_H:               pIO.AddKeyEvent(ImGuiKey_H, pPressed); break;
		case IMGN_KEY_I:               pIO.AddKeyEvent(ImGuiKey_I, pPressed); break;
		case IMGN_KEY_J:               pIO.AddKeyEvent(ImGuiKey_J, pPressed); break;
		case IMGN_KEY_K:               pIO.AddKeyEvent(ImGuiKey_K, pPressed); break;
		case IMGN_KEY_L:               pIO.AddKeyEvent(ImGuiKey_L, pPressed); break;
		case IMGN_KEY_M:               pIO.AddKeyEvent(ImGuiKey_M, pPressed); break;
		case IMGN_KEY_N:               pIO.AddKeyEvent(ImGuiKey_N, pPressed); break;
		case IMGN_KEY_O:               pIO.AddKeyEvent(ImGuiKey_O, pPressed); break;
		case IMGN_KEY_P:               pIO.AddKeyEvent(ImGuiKey_P, pPressed); break;
		case IMGN_KEY_Q:               pIO.AddKeyEvent(ImGuiKey_Q, pPressed); break;
		case IMGN_KEY_R:               pIO.AddKeyEvent(ImGuiKey_R, pPressed); break;
		case IMGN_KEY_S:               pIO.AddKeyEvent(ImGuiKey_S, pPressed); break;
		case IMGN_KEY_T:               pIO.AddKeyEvent(ImGuiKey_T, pPressed); break;
		case IMGN_KEY_U:               pIO.AddKeyEvent(ImGuiKey_U, pPressed); break;
		case IMGN_KEY_V:               pIO.AddKeyEvent(ImGuiKey_V, pPressed); break;
		case IMGN_KEY_W:               pIO.AddKeyEvent(ImGuiKey_W, pPressed); break;
		case IMGN_KEY_X:               pIO.AddKeyEvent(ImGuiKey_X, pPressed); break;
		case IMGN_KEY_Y:               pIO.AddKeyEvent(ImGuiKey_Y, pPressed); break;
		case IMGN_KEY_Z:               pIO.AddKeyEvent(ImGuiKey_Z, pPressed); break;

		case IMGN_KEY_F1:              pIO.AddKeyEvent(ImGuiKey_F1, pPressed); break;
		case IMGN_KEY_F2:              pIO.AddKeyEvent(ImGuiKey_F2, pPressed); break;
		case IMGN_KEY_F3:              pIO.AddKeyEvent(ImGuiKey_F3, pPressed); break;
		case IMGN_KEY_F4:              pIO.AddKeyEvent(ImGuiKey_F4, pPressed); break;
		case IMGN_KEY_F5:              pIO.AddKeyEvent(ImGuiKey_F5, pPressed); break;
		case IMGN_KEY_F6:              pIO.AddKeyEvent(ImGuiKey_F6, pPressed); break;
		case IMGN_KEY_F7:              pIO.AddKeyEvent(ImGuiKey_F7, pPressed); break;
		case IMGN_KEY_F8:              pIO.AddKeyEvent(ImGuiKey_F8, pPressed); break;
		case IMGN_KEY_F9:              pIO.AddKeyEvent(ImGuiKey_F9, pPressed); break;
		case IMGN_KEY_F10:             pIO.AddKeyEvent(ImGuiKey_F10, pPressed); break;
		case IMGN_KEY_F11:             pIO.AddKeyEvent(ImGuiKey_F11, pPressed); break;
		case IMGN_KEY_F12:             pIO.AddKeyEvent(ImGuiKey_F12, pPressed); break;

		case IMGN_KEY_APOSTROPHE:      pIO.AddKeyEvent(ImGuiKey_Apostrophe, pPressed); break;
		case IMGN_KEY_COMMA:           pIO.AddKeyEvent(ImGuiKey_Comma, pPressed); break;
		case IMGN_KEY_MINUS:           pIO.AddKeyEvent(ImGuiKey_Minus, pPressed); break;
		case IMGN_KEY_PERIOD:          pIO.AddKeyEvent(ImGuiKey_Period, pPressed); break;
		case IMGN_KEY_SLASH:           pIO.AddKeyEvent(ImGuiKey_Slash, pPressed); break;
		case IMGN_KEY_SEMICOLON:       pIO.AddKeyEvent(ImGuiKey_Semicolon, pPressed); break;
		case IMGN_KEY_EQUAL:           pIO.AddKeyEvent(ImGuiKey_Equal, pPressed); break;
		case IMGN_KEY_LEFT_BRACKET:    pIO.AddKeyEvent(ImGuiKey_LeftBracket, pPressed); break;
		case IMGN_KEY_BACKSLASH:       pIO.AddKeyEvent(ImGuiKey_Backslash, pPressed); break;
		case IMGN_KEY_RIGHT_BRACKET:   pIO.AddKeyEvent(ImGuiKey_RightBracket, pPressed); break;
		case IMGN_KEY_GRAVE_ACCENT:    pIO.AddKeyEvent(ImGuiKey_GraveAccent, pPressed); break;

		case IMGN_KEY_CAPS_LOCK:       pIO.AddKeyEvent(ImGuiKey_CapsLock, pPressed); break;
		case IMGN_KEY_SCROLL_LOCK:     pIO.AddKeyEvent(ImGuiKey_ScrollLock, pPressed); break;
		case IMGN_KEY_NUM_LOCK:        pIO.AddKeyEvent(ImGuiKey_NumLock, pPressed); break;
		case IMGN_KEY_PRINT_SCREEN:    pIO.AddKeyEvent(ImGuiKey_PrintScreen, pPressed); break;
		case IMGN_KEY_PAUSE:           pIO.AddKeyEvent(ImGuiKey_Pause, pPressed); break;

		case IMGN_KEY_KP_0:            pIO.AddKeyEvent(ImGuiKey_Keypad0, pPressed); break;
		case IMGN_KEY_KP_1:            pIO.AddKeyEvent(ImGuiKey_Keypad1, pPressed); break;
		case IMGN_KEY_KP_2:            pIO.AddKeyEvent(ImGuiKey_Keypad2, pPressed); break;
		case IMGN_KEY_KP_3:            pIO.AddKeyEvent(ImGuiKey_Keypad3, pPressed); break;
		case IMGN_KEY_KP_4:            pIO.AddKeyEvent(ImGuiKey_Keypad4, pPressed); break;
		case IMGN_KEY_KP_5:            pIO.AddKeyEvent(ImGuiKey_Keypad5, pPressed); break;
		case IMGN_KEY_KP_6:            pIO.AddKeyEvent(ImGuiKey_Keypad6, pPressed); break;
		case IMGN_KEY_KP_7:            pIO.AddKeyEvent(ImGuiKey_Keypad7, pPressed); break;
		case IMGN_KEY_KP_8:            pIO.AddKeyEvent(ImGuiKey_Keypad8, pPressed); break;
		case IMGN_KEY_KP_9:            pIO.AddKeyEvent(ImGuiKey_Keypad9, pPressed); break;
		case IMGN_KEY_KP_DECIMAL:      pIO.AddKeyEvent(ImGuiKey_KeypadDecimal, pPressed); break;
		case IMGN_KEY_KP_DIVIDE:       pIO.AddKeyEvent(ImGuiKey_KeypadDivide, pPressed); break;
		case IMGN_KEY_KP_MULTIPLY:     pIO.AddKeyEvent(ImGuiKey_KeypadMultiply, pPressed); break;
		case IMGN_KEY_KP_SUBTRACT:     pIO.AddKeyEvent(ImGuiKey_KeypadSubtract, pPressed); break;
		case IMGN_KEY_KP_ADD:          pIO.AddKeyEvent(ImGuiKey_KeypadAdd, pPressed); break;
		case IMGN_KEY_KP_ENTER:        pIO.AddKeyEvent(ImGuiKey_KeypadEnter, pPressed); break;
			//case IMGN_KEY_KP_EQUAL:        pIO.AddKeyEvent(ImGuiKey_KeypadEqual, pPressed); break;
		//mouse
		case IMGN_MOUSE_BUTTON_LEFT:	pIO.AddMouseButtonEvent(ImGuiMouseButton_Left, pPressed); break;
		case IMGN_MOUSE_BUTTON_RIGHT:	pIO.AddMouseButtonEvent(ImGuiMouseButton_Right, pPressed); break;
		case IMGN_MOUSE_BUTTON_MIDDLE:	pIO.AddMouseButtonEvent(ImGuiMouseButton_Middle, pPressed); break;
		default:
			break;
		}

	}

	int ImGuiLayer::ImGui_ImplWin32_CreateVkSurface(ImGuiViewport* pViewport, ImU64 pVkInstance, const void* pVkAllocator, ImU64* pOutSurface)
	{
		VkWin32SurfaceCreateInfoKHR createInfo
		{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = GetModuleHandle(nullptr),
			.hwnd = (HWND)pViewport->PlatformHandleRaw
		};

		return (int)vkCreateWin32SurfaceKHR((VkInstance)pVkInstance, &createInfo, (const VkAllocationCallbacks*)pVkAllocator, (VkSurfaceKHR*)pOutSurface);
	}

	void ImGuiLayer::Sleep()
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		//io.Fonts->AddFontFromFileTTF("../../Fonts/Raleway-Regular.ttf", 16.f);
		io.Fonts->AddFontFromFileTTF("../../Fonts/consola.ttf", 12.f);
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplWin32_Init(_window->GetWindowHandle());
		ImGui::GetPlatformIO().Platform_CreateVkSurface = ImGui_ImplWin32_CreateVkSurface;

		ImGui_ImplVulkan_InitInfo initInfo = _renderer->GetImGuiInitInfo();

		ImGui_ImplVulkan_Init(&initInfo);
	}

	void ImGuiLayer::WakeUp()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnImGuiRender()
	{
		//ImGui::DockSpaceOverViewport();

		//// Show demo options and help
		//if (ImGui::BeginMainMenuBar())
		//{
		//	if (ImGui::BeginMenu("File"))
		//	{
		//		if (ImGui::MenuItem("Exit")) ImgnApp::Get().Close();
		//		ImGui::EndMenu();
		//	}
		//	ImGui::EndMainMenuBar();
		//}

		//ImGui::ShowDemoWindow();
		//ImGui::Begin("Style");
		//ImGui::ShowStyleEditor();
		//ImGui::End();

		//if (!_sceneWindow)
		//{
		//	_sceneWindow = static_cast<vk::DescriptorSet>(ImGui_ImplVulkan_AddTexture(*_renderer->GetTextureSampler(), **_renderer->GetRenderGraphImage("LitScene").image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
		//}

		//ImTextureID textureID = static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(static_cast<VkDescriptorSet>(_sceneWindow)));

		//ImGui::Begin("Vulkan Texture Test");
		//ImGui::Image(ImTextureRef(textureID), ImVec2(_window->GetWidth(), _window->GetHeight()));
		//ImGui::End();

		//ImGui::End();
	}

	void ImGuiLayer::Dream(Time pTime)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = pTime;	
	}

	void ImGuiLayer::OnEvent(Event& pEvent)
	{
		EventDispatcher dispatcher(pEvent);

		dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& e)
			{
				ImGuiIO& io = ImGui::GetIO();
				AddImGuiSpecialInputEvent(io, e.GetKeyCode(), true);

				return false;
			});
		dispatcher.Dispatch<KeyReleasedEvent>([&](KeyReleasedEvent& e)
			{
				ImGuiIO& io = ImGui::GetIO();
				AddImGuiSpecialInputEvent(io, e.GetKeyCode(), false);

				return false;
			});

		dispatcher.Dispatch<KeyTypedEvent>([&](KeyTypedEvent& e)
			{
				ImGuiIO& io = ImGui::GetIO();
				int keyCode = e.GetKeyCode();

				if (keyCode > 0 && keyCode < 0x10000) io.AddInputCharacter(keyCode);

				return false;
			});

		dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e)
			{
				ImGuiIO& io = ImGui::GetIO();
				AddImGuiSpecialInputEvent(io, e.GetMouseButton(), true);

				return false;
			});
		dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e)
			{
				ImGuiIO& io = ImGui::GetIO();
				AddImGuiSpecialInputEvent(io, e.GetMouseButton(), false);

				return false;
			});
		dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e)
			{
				ImGuiIO& io = ImGui::GetIO();
				io.MousePos = ImVec2(e.GetX(), e.GetY());

				return false;
			});

		dispatcher.Dispatch<MouseScrolledEvent>([&](MouseScrolledEvent& e)
			{
				ImGuiIO& io = ImGui::GetIO();
				io.MouseWheelH += e.GetXOffset();
				io.MouseWheel += e.GetYOffset();

				return false;
			});

		/*dispatcher.Dispatch<WindowResizedEvent>([&](WindowResizedEvent& e)
			{
				ImGuiIO& io = ImGui::GetIO();
				io.DisplaySize = ImVec2(e.GetWidth(), e.GetHeight());
				io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

				return false;
			});*/

	}
	void ImGuiLayer::Begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(_window->GetWidth(), _window->GetHeight());
		//io.DeltaTime = pTime;
		ImGui::Render();
		vk::raii::CommandBuffer& commandBuffer = _renderer->GetActiveCommandBuffer();
		vk::RenderingAttachmentInfo colorAttachment
		{
			.imageView = _renderer->GetActiveSwapchainImageView(),
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eLoad,
			.storeOp = vk::AttachmentStoreOp::eStore
		};

		vk::RenderingInfo renderingInfo
		{
			.renderArea =
			{
				.offset = { 0, 0 },
				.extent = _renderer->GetSwapchainExtent()
			},
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachment
		};

		commandBuffer.beginRendering(renderingInfo);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *commandBuffer);

		commandBuffer.endRendering();

	}
}