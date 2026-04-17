#pragma once

class Entity;

class ComponentTypeIDSystem
{
	static uint64_t _nextTypeID;

public:
	template<typename T>
	static size_t GetTypeID()
	{
		static size_t typeID = _nextTypeID++;
		return typeID;
	}
};

class Component
{
	friend class Entity;

public:
	enum class State
	{
		Uninitialized,
		Initializing,
		Active,
		Destroying,
		Destroyed
	};

private:
	std::unique_ptr<Entity> _owner = nullptr;
	State _state = State::Uninitialized;

	virtual void OnInit() {}
	virtual void Update(float pDeltaTime) {}
	virtual void Render() {}
	virtual void OnDestroy() {}

public:
	virtual ~Component()
	{
		if (_state != State::Destroyed)
		{
			OnDestroy();
			_state = State::Destroyed;
		}
	}

	virtual void Init()
	{
		if (_state == State::Uninitialized)
		{
			_state = State::Initializing;
			OnInit();
			_state = State::Active;
		}
	}

	void SetOwner(Entity* pEntity) { _owner.reset(pEntity); }
	Entity* GetOwner() const { return _owner.get(); }

	template<typename T>
	static uint64_t GetTypeID()
	{
		return ComponentTypeIDSystem::GetTypeID<T>();
	}
};

class Entity
{
	std::string _name;
	bool _active = true;
	std::vector<std::unique_ptr<Component>> _components;
	std::unordered_map<uint64_t, Component*> _componentMap;

public:
	explicit Entity(const std::string& pName)
	{
		_name = pName;
	}

	const std::string& GetName() const { return _name; }
	bool IsActive() const { return _active; }
	void SetActive(bool pIsActive) { _active = pIsActive; }

	void Init();
	void Update(float pDeltaTime);
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
		std::unique_ptr<T> component = std::make_unique<T>(std::forward<Args>(pArgs)...);
		T* compPtr = component.get();
		_componentMap[typeID] = compPtr;
		compPtr->SetOwner(this);
		_components.push_back(std::move(component));
		
		return compPtr;
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