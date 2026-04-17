#pragma once

#include "ImgnWindow.h"
#include "Events/Event.h"
#include "Events/AppEvent.h"

namespace IMGN
{
	class ImgnApp
	{
		//imgnwindow needs to use gw... for now
		unique<ImgnWindow> _window;
		bool _running = true;

		std::vector<Imagination*> _imaginations;

	public:
		ImgnApp()
		{

		}

		virtual ~ImgnApp()
		{

		}

		void Run();
		void OnEvent(Event& pEvent);

	};

	ImgnApp* CreateApp();
}