#pragma once

class Entity;

class Component
{

protected:
	std::unique_ptr<Entity> _owner = nullptr;

public:
	virtual ~Component() = default;

	virtual void Init() {}
	virtual void Update(float pDeltaTime) {}
	virtual void Render() {}

	void SetOwner(Entity* pEntity) { _owner.reset(pEntity); }
	Entity* GetOwner() const { return _owner.get(); }
};

class Entity
{
	std::string _name;
	bool _active = true;
	std::vector<std::unique_ptr<Component>> _components;

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

		std::unique_ptr<T> component = std::make_unique<T>(std::forward<Args>(pArgs)...);
		T* compPtr = component.get();
		compPtr->SetOwner(this);
		_components.push_back(std::move(component));
		
		return compPtr;
	}

	template<typename T>
	T* GetComponent()
	{
		for (auto& component : _components)
		{
			if (T* result = dynamic_cast<T*>(component.get()))
			{
				return result;
			}
		}

		return nullptr;
	}

	template<typename T>
	bool RemoveComponent()
	{
		for (auto it = _components.begin(); it != _components.end(); ++it)
		{
			if (dynamic_cast<T*>(it->get()))
			{
				_components.erase(it);
				return true;
			}
		}
		return false;
	}
};