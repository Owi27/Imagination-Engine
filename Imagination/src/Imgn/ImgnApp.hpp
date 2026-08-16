#pragma once
#include "ImgnCore.hpp"
#include "ImgnWindow.h"
#include "Events/Event.h"
#include "ImgnComponent.h"
#include "ImgnGLTF.h"
#include "ImgnLayer.h"
#include "ImgnLayerStack.h"
#include "ImGui/ImgnGui.h"
#include "ImgnInput.h"

namespace Imgn
{
	class IMGN_API ImgnApp
	{
		static inline ImgnApp* _instance;

		bool _running = true;
		LayerStack _layerStack;
		std::chrono::steady_clock::time_point _lastFrameTime;
		unique<ImgnWindow> _window;
		unique<ImGuiLayer> _imGuiLayer;
		ImgnEntity _entity;
		unique<ImgnRenderer> _renderer;

	public:
		ImgnApp()
		{
			if (_instance) IMGN_FATAL("Only one app instance allowed");
			_instance = this;

			//if (!_instance) _instance.reset(new ImgnApp());

			_window = Unique<ImgnWindow>();
			_window->SetEventCallback(IMGN_BIND_EVENT_FN(ImgnApp::OnEvent));
			Input::Get(); //calling this just to create the input instance
			Input::AttachToWindow(_window->GetUniversalWindowHandle());
			Input::SetEventCallback(IMGN_BIND_EVENT_FN(ImgnApp::OnEvent));

			_renderer = Unique<ImgnRenderer>();

			_renderer->Init(_window->GetRendererCreateInfo());
			IMGN_CORE_INFO("Graphics ctx initialized");

			_imGuiLayer = Unique<ImGuiLayer>(&*_window, &*_renderer);
			_imGuiLayer->Sleep();

			//ImgnGLTF gl;
			//gl.LoadModel("../../Models/Sponza/glTF/Sponza.gltf", r);
		}

		virtual ~ImgnApp()
		{
			_instance = nullptr;
		}

		inline static ImgnApp& Get()
		{
			return *_instance;
		}

		inline ImgnWindow& GetWindow() { return *_window; }
		inline ImgnRenderer& Renderer() { return *_renderer; }

		void Run();
		void OnEvent(Event& pEvent);

		void AddLayer(unique<Layer> pLayer);
		void AddOverlay(unique<Layer> pLayer);

		void Close() { _running = false; }

		template<typename T, typename... Args>
		T* AddComponent(Args&&... pArgs)
		{
			return _entity.AddComponent<T>(std::forward<Args>(pArgs)...);
		}

		template<typename T>
		void RemoveComponent()
		{
			_entity.RemoveComponent<T>();
		}
	};

	ImgnApp* CreateApplication();
}