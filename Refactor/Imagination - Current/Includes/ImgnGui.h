#pragma once
#include "ResourceTypes.h"
#include "ImgnWindow.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_vulkan.h"
#include <Windows.h>

#include "Events/Event.h"
#include "Events/AppEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

class ImgnGui : public EventListener
{
	struct ImGuiPushConst
	{
		vec2 scale, translate;
	} _imGuiPushConst;

	Image _font;
	Buffer _vertex, _idx;
	vk::raii::Sampler _sampler = nullptr;
	uint32_t _vertexCount = 0, _idxCount = 0;

	vk::raii::PipelineCache _pipelineCache = nullptr;
	vk::raii::PipelineLayout _pipelineLayout = nullptr;
	vk::raii::Pipeline _pipeline = nullptr;
	vk::raii::DescriptorPool _descriptorPool = nullptr;
	vk::raii::DescriptorSetLayout _descriptorSetLayout = nullptr;
	vk::raii::DescriptorSet _descriptorSet = nullptr;
	vk::PipelineRenderingCreateInfo _renderingInfo = {};

	GBufferedInput* _bufferedInput;

	bool _needsUpdateBuffers = false;
	vk::Format _colorFormat = vk::Format::eB8G8R8A8Unorm;

	void CreateResources();
	uint32_t FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps);

	GEventResponder responder;

	//events
	void OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);
	void OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);
	void OnMouseMovedEvent(MouseMovedEvent& e);
	void OnMouseScrolledEvent(MouseScrolledEvent& e);
	void OnKeyPressedEvent(KeyPressedEvent& e);
	void OnKeyReleasedEvent(KeyReleasedEvent& e);
	void OnKeyTypedEvent(KeyTypedEvent& e);
	void OnWindowResizeEvent(WindowResizedEvent& e);

public:
	ImgnGui()
	{
		responder.Create([&](const GEvent& e)
			{
				GBufferedInput::Events event;
				GBufferedInput::EVENT_DATA data;

				if (+e.Read(event, data))
				{
					switch (event)
					{
					case GBufferedInput::Events::Invalid:
						break;
					case GBufferedInput::Events::KEYPRESSED:
						{
							KeyPressedEvent kpe(data.keyMask, 1);
							EventDispatcher dispatcher(kpe);
							dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& e)
								{
									ImGuiIO& io = ImGui::GetIO();
									io.KeysData[e.GetKeyCode()].Down = true;

									io.KeyCtrl = io.KeysData[G_KEY_LEFTCONTROL].Down || io.KeysData[G_KEY_RIGHTCONTROL].Down;
									io.KeyShift = io.KeysData[G_KEY_LEFTSHIFT].Down || io.KeysData[G_KEY_RIGHTSHIFT].Down;
									io.KeyAlt = io.KeysData[G_KEY_LEFTALT].Down || io.KeysData[G_KEY_RIGHTALT].Down;
									io.KeySuper = io.KeysData[G_KEY_COMMAND].Down;

									return false;
								});
						}
						break;
					case GBufferedInput::Events::KEYRELEASED:
						{
							KeyReleasedEvent kpe(data.keyMask);
							EventDispatcher dispatcher(kpe);
							dispatcher.Dispatch<KeyReleasedEvent>([&](KeyReleasedEvent& e)
								{
									ImGuiIO& io = ImGui::GetIO();
									io.KeysData[e.GetKeyCode()].Down = false;


									return false;
								});
						}
						break;
					case GBufferedInput::Events::BUTTONPRESSED:
						{
							MouseButtonPressedEvent mme(data.data);
							EventDispatcher dispatcher(mme);
							dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e)
								{
									ImGuiIO& io = ImGui::GetIO();
									io.MouseDown[e.GetMouseButton()] = true;

									return false;
								});
						}
						break;
					case GBufferedInput::Events::BUTTONRELEASED:
						{
							MouseButtonReleasedEvent mme(data.data);
							EventDispatcher dispatcher(mme);
							dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e)
								{
									ImGuiIO& io = ImGui::GetIO();
									io.MouseDown[e.GetMouseButton()] = false;


									return false;
								});
						}
						break;
					case GBufferedInput::Events::MOUSEMOVE:
						{
							MouseMovedEvent mme(data.x, data.y);
							EventDispatcher dispatcher(mme);
							dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e)
								{
									ImGuiIO& io = ImGui::GetIO();
									io.MousePos = ImVec2(e.GetX(), e.GetY());

									return false;
								});

						}
						break;
					case GBufferedInput::Events::MOUSESCROLL:
						{
							MouseScrolledEvent me(0, 0); //todo
							EventDispatcher dispatcher(me);
							dispatcher.Dispatch<MouseScrolledEvent>([&](MouseScrolledEvent& e)
								{
									ImGuiIO& io = ImGui::GetIO();
									io.MouseWheelH += e.GetXOffset();
									io.MouseWheel += e.GetYOffset();

									return false;
								});
						}
						break;
					}
				}

			});
	}

	~ImgnGui()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void Init(ImgnWindow& pWin, float pWidth, float pHeight);
	void Init(HWND pWin, float pWidth, float pHeight);

	void SetInput(GBufferedInput* pBufferedInput) { _bufferedInput = pBufferedInput; _bufferedInput->Register(responder); }
	//bool NewFrame();
	//void UpdateBuffers();
	void DrawFrame(vk::raii::CommandBuffer& pCommandBuffer);

	// Inherited via EventListener
	void OnEvent(Event& event) override;
};

