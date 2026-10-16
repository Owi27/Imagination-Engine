#pragma once
#include "Imgn/ImgnComponent.h"

namespace Imgn
{
	class IMGN_API ScriptComponent : public Component
	{
	public:
		IMGN_COMPONENT_ID("Imgn.ScriptComponent");

		unique<ScriptableEntity> instance = nullptr;

		std::function<void()> Create;
		std::function<void()> Destroy;
		std::function<void()> Sleep;
		std::function<void()> WakeUp;
		std::function<void(Time pTime)> Dream;

		template<typename T> requires std::derived_from<T, ScriptableEntity>
		void Bind()
		{
			Create = [&]() { instance = Unique<T>(); };
			Sleep = [&]() { static_cast<T*>(instance.get())->Sleep(); };
			WakeUp = [&]() { static_cast<T*>(instance.get())->WakeUp(); };
			Dream = [&](Time pTime) { static_cast<T*>(instance.get())->Dream(pTime); };
		}

		void Serialize(std::fstream& pStream) override;
		void Deserialize(std::fstream& pStream) override;
	};
}