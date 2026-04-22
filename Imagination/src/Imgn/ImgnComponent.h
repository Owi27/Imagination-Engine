#pragma once
#include "ImgnCore.hpp"
#include "Events/Event.h"
class ImgnEntity;

class ImgnComponentTypeIDSystem
{
	inline static uint64_t _nextTypeID;

public:
	template<typename T>
	static uint64_t GetTypeID()
	{
		static uint64_t typeID = _nextTypeID++;
		return typeID;
	}
};

class IMGN_API ImgnComponent
{
	enum class State;
	friend class ImgnEntity;

	unique<ImgnEntity> _owner = nullptr;
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
	ImgnComponent(const std::string& pName = "ImgnComponent")
	{
		_debugName = pName;
	}

	virtual ~ImgnComponent()
	{
		if (_state != State::Destroyed)
		{
			OnDestroy();
			_state = State::Destroyed;
		}
	}

	/* Class Functions */
	void SetOwner(ImgnEntity* pEntity) { _owner.reset(pEntity); }
	ImgnEntity* GetOwner() const { return _owner.get(); }
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
	static uint64_t GetTypeID()
	{
		return ImgnComponentTypeIDSystem::GetTypeID<T>();
	}
};

class ImgnEntity
{
	std::string _name;
	bool _active = true;
	std::vector<unique<ImgnComponent>> _components;
	std::unordered_map<uint64_t, ImgnComponent*> _componentMap;

public:
	ImgnEntity(const std::string& pName = "ImgnEntity")
	{
		_name = pName;
	}

	~ImgnEntity()
	{

	}

	std::vector<unique<ImgnComponent>>::iterator begin() { return _components.begin(); }
	std::vector<unique<ImgnComponent>>::iterator end() { return _components.end(); }

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
		static_assert(std::is_base_of<ImgnComponent, T>::value, "T must derive from Component");

		uint64_t typeID = ImgnComponent::GetTypeID<T>();

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
	T* GetComponent()
	{
		uint64_t typeID = ImgnComponent::GetTypeID<T>();
		auto it = _componentMap.find(typeID);
		if (it != _componentMap.end()) return static_cast<T*>(it->second);

		return nullptr;
	}

	template<typename T>
	bool RemoveComponent()
	{
		uint64_t typeID = ImgnComponent::GetTypeID<T>();
		auto it = _componentMap.find(typeID);
		if (it != _componentMap.end())
		{
			ImgnComponent* compPtr = it->second;
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