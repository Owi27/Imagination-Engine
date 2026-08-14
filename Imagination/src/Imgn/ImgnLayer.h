#pragma once
#include "Events/Event.h"
#include "ImgnTime.h"

namespace Imgn
{
	class IMGN_API Layer
	{
	protected:
		std::string _name;

	public:
		Layer(const std::string& pName = "Layer")
		{
			_name = pName;
		}

		virtual ~Layer()
		{

		}

		virtual void Sleep() {}
		virtual void WakeUp() {}
		//virtual void OnImGuiRender() {}
		virtual void Dream(Time pTime) {}
		virtual void OnEvent(Event& pEvent) {}

		inline const std::string& GetName() const { return _name; }
	};

}