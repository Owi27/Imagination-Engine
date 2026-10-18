#pragma once
#include "Imgn/ImgnComponent.h"

namespace Imgn
{
	class IMGN_API ScriptComponent : public Component
	{
		std::function<void(ScriptComponent*)> _bind;

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
			_bind = [](ScriptComponent* pComponent) { pComponent->Bind<T>(); };
			Create = [&]() { instance = Unique<T>(); };
			Sleep = [&]() { static_cast<T*>(instance.get())->Sleep(); };
			WakeUp = [&]() { static_cast<T*>(instance.get())->WakeUp(); };
			Dream = [&](Time pTime) { static_cast<T*>(instance.get())->Dream(pTime); };
		}

		bool CopyBindingFrom(const ScriptComponent& pSource)
		{
			if (instance || !pSource._bind) return false;

			pSource._bind(this);
			return true;
		}

		void Serialize(std::fstream& pStream) override;
		void Deserialize(std::fstream& pStream) override;
	};
}