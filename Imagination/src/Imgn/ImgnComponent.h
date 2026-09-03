#pragma once
#include "ImgnCore.hpp"
#include "Events/Event.h"
#include "ImgnTime.h"

#define IMGN_COMPONENT_ID(pTypeName) static constexpr ID TypeID = HashID(pTypeName);

namespace Imgn
{
	class Entity;
	using ID = uint64_t;
	constexpr ID HashID(std::string_view pName)
	{
		ID hash = 14695981039346656037ull;

		for (char c : pName)
		{
			hash ^= static_cast<unsigned char>(c);

			hash *= 1099511628211ull;
		}

		return hash;
	}

	IMGN_API ID GenerateEntityID(std::string_view pName);

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
		static constexpr ID GetTypeID()
		{
			return T::TypeID;
		}

		virtual void Serialize(std::fstream& pStream) = 0;
		virtual void Deserialize(std::fstream& pStream) = 0;
	};

	class Entity
	{
		ID _id = 0;
		std::string _name;
		bool _active = true;
		std::vector<unique<Component>> _components;
		std::unordered_map<uint64_t, Component*> _componentMap;

	public:
		Entity(const std::string& pName = "ImgnEntity")
		{
			_name = pName;
			_id = GenerateEntityID(_name);
		}

		~Entity()
		{

		}

		Entity(const Entity&) = delete;
		Entity& operator=(const Entity&) = delete;

		std::vector<unique<Component>>::iterator begin() { return _components.begin(); }
		std::vector<unique<Component>>::iterator end() { return _components.end(); }

		ID GetID() const { return _id; }
		const std::string& GetName() const { return _name; }
		void SetName(const std::string& pNewName) { _name = pNewName; }
		bool IsActive() const { return _active; }
		void SetActive(bool pIsActive) { _active = pIsActive; }
		const std::vector<unique<Component>>& GetComponents() const { return _components; }

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

		bool operator==(const Entity& pOther) const { return _name == pOther._name; }
		bool operator!=(const Entity& pOther) const { return !(*this == pOther); }
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