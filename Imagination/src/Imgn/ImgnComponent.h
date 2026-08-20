#pragma once
#include "ImgnCore.hpp"
#include "Events/Event.h"

namespace Imgn
{
	class Entity;
	using ComponentTypeID = uint64_t;
	constexpr ComponentTypeID HashComponentName(std::string_view pName)
	{
		ComponentTypeID hash = 14695981039346656037ull;

		for (char c : pName)
		{
			hash ^= static_cast<unsigned char>(c);

			hash *= 1099511628211ull;
		}

		return hash;
	}

	class IMGN_API Component
	{
		enum class State;
		friend class Entity;

		Entity* _owner = nullptr;
		State _state = State::Uninitialized;

		virtual void OnInit() {}
		virtual void Dream(float pDeltaTime) {}
		virtual void OnEvent(Event& pEvent) {}
		virtual void OnDestroy() {}

		virtual void Render() {}

	protected:
		std::string _debugName;

	public:
		enum class State
		{
			Uninitialized,
			Initializing,
			Active,
			Destroying,
			Destroyed
		};

		/* Class Defaults */
		Component(const std::string& pName = "ImgnComponent")
		{
			_debugName = pName;
		}

		virtual ~Component()
		{
			if (_state != State::Destroyed)
			{
				OnDestroy();
				_state = State::Destroyed;
			}
		}

		/* Class Functions */
		void SetOwner(Entity* pEntity) { _owner = pEntity; }
		Entity* GetOwner() const { return _owner; }
		inline const std::string& GetName() const { return _debugName; }

		virtual void Init()
		{
			if (_state == State::Uninitialized)
			{
				_state = State::Initializing;
				OnInit();
				_state = State::Active;
			}
		}

		template<typename T>
		static ComponentTypeID GetTypeID()
		{
			return T::TypeID;
		}
	};

	class Entity
	{
		std::string _name;
		bool _active = true;
		std::vector<unique<Component>> _components;
		std::unordered_map<uint64_t, Component*> _componentMap;

	public:
		Entity(const std::string& pName = "ImgnEntity")
		{
			_name = pName;
		}

		~Entity()
		{

		}

		Entity(const Entity&) = delete;
		Entity& operator=(const Entity&) = delete;

		std::vector<unique<Component>>::iterator begin() { return _components.begin(); }
		std::vector<unique<Component>>::iterator end() { return _components.end(); }

		const std::string& GetName() const { return _name; }
		bool IsActive() const { return _active; }
		void SetActive(bool pIsActive) { _active = pIsActive; }

		void Init();
		void Dream(float pDeltaTime);
		void OnEvent(Event& pEvent);
		void Render();

		template<typename T, typename... Args>
		T* AddComponent(Args&&... pArgs)
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
			uint64_t typeID = Component::GetTypeID<T>();

			//check if component of this type already exists
			auto it = _componentMap.find(typeID);
			if (it != _componentMap.end()) return static_cast<T*>(it->second);

			//create new component
			unique<T> component = std::make_unique<T>(std::forward<Args>(pArgs)...);
			T* compPtr = component.get();
			_componentMap[typeID] = compPtr;
			compPtr->SetOwner(this);
			_components.push_back(std::move(component));

			return compPtr;
		}

		template<typename T>
		bool HasComponent()
		{
			uint64_t typeID = Component::GetTypeID<T>();
			return _componentMap.contains(typeID);
		}

		template<typename T>
		T* GetComponent()
		{
			uint64_t typeID = Component::GetTypeID<T>();
			auto it = _componentMap.find(typeID);
			if (it != _componentMap.end()) return static_cast<T*>(it->second);

			return nullptr;
		}

		template<typename T>
		bool RemoveComponent()
		{
			uint64_t typeID = Component::GetTypeID<T>();
			auto it = _componentMap.find(typeID);
			if (it != _componentMap.end())
			{
				Component* compPtr = it->second;
				_componentMap.erase(it);

				for (auto compIt = _components.begin(); compIt != _components.end(); ++compIt)
				{
					if (compIt->get() == compPtr)
					{
						_components.erase(compIt);
						return true;
					}
				}
			}

			return false;
		}
	};

	class IMGN_API ScriptableEntity
	{
		friend class Scene;
		Entity* _entity;

	public:
		ScriptableEntity() /*Constructor*/
		{
		}

		~ScriptableEntity() /*Destructor*/
		{
		}

		/*Copy Constructor*/
		ScriptableEntity(const ScriptableEntity& pOther) = default;

		/*Copy Assignment Operator*/
		ScriptableEntity& operator=(const ScriptableEntity& pOther) = default;

		/*Move Constructor*/
		ScriptableEntity(ScriptableEntity&& pOther) noexcept = default;

		/*Move Assignment Operator*/
		ScriptableEntity& operator=(ScriptableEntity&& pOther) noexcept = default;

		/*Class Functions*/
		template<typename T>
		T* GetComponent()
		{
			return _entity->GetComponent<T>();
		}
	};
}