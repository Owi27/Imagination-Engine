#pragma once
#include "ImgnCore.hpp"
#include "ImgnWindow.h"
#include "Events/Event.h"
#include "ImgnComponent.h"
#include "ImgnGLTF.h"

class IMGN_API ImgnApp
{
	static inline unique<ImgnApp> _instance;

	bool _running = true;
	unique<ImgnWindow> _window;
	ImgnEntity _entity;
	unique<ImgnRenderer> _renderer;

public:
	ImgnApp()
	{
		//if (!_instance) _instance.reset(new ImgnApp());

		_window = Unique<ImgnWindow>();
		_renderer = Unique<ImgnRenderer>();
		
		_renderer->Init(_window->GetRendererCreateInfo());
		IMGN_CORE_INFO("Graphics ctx initialized");


		//ImgnGLTF gl;
		//gl.LoadModel("../../Models/Sponza/glTF/Sponza.gltf", r);
	}

	~ImgnApp()
	{

	}

	inline static ImgnApp& Get() 
	{ 
		if (!_instance) _instance.reset(new ImgnApp()); 
		return *_instance; 
	}
	inline ImgnWindow& GetWindow() { return *_window; }
	inline ImgnRenderer& Renderer() { return *_renderer; }

	void Run();
	void OnEvent(Event& pEvent);

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

namespace IMGN
{
	ImgnApp* CreateApplication();
}