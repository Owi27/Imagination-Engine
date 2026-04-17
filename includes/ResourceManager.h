#pragma once

class Resource
{
	std::string _ID;
	bool _loaded = false;

protected:
	virtual bool DoLoad() = 0;
	virtual bool DoUnload() = 0;

public:
	explicit Resource(const std::string& pID)
	{
		_ID = pID;
	}

	virtual ~Resource() = default;

	const std::string& GetID() const { return _ID; }
	bool IsLoaded() const { return _loaded; }

	virtual bool Load();
	virtual void Unload();
};

template<typename T>
class ResourceHandle;

class ResourceManager
{
	struct ResourceData
	{
		std::shared_ptr<Resource> resource;
		int refCount;
	};

	std::unordered_map<std::type_index, std::unordered_map<std::string, ResourceData>> _refCounts;
	std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<Resource>>> _resources;

public:
	template<typename T>
	ResourceHandle<T> Load(const std::string& pID)
	{
		static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

		auto& typeResources = _resources[std::type_index(typeid(T))];
		auto it = typeResources.find(pID);

		if (it != typeResources.end())
		{
			_refCounts[std::type_index(typeid(T))][pID].refCount++;

			return ResourceHandle<T>(pID, this);
		}

		auto resource = std::make_shared<T>(pID);
		if (!resource->Load())
		{
			// Loading failed - return invalid handle rather than corrupting cache
			return ResourceHandle<T>();
		}

		typeResources[pID] = resource;
		_refCounts[std::type_index(typeid(T))][pID].refCount = 1;

		return ResourceHandle<T>(pID, this);
	}

	template<typename T>
	T* GetResource(const std::string& pID)
	{
		// Access type-specific resource container using compile-time type information
		auto& typeResources = _resources[std::type_index(typeid(T))];
		auto it = typeResources.find(pID);

		if (it != typeResources.end())
		{
			// Resource found - perform safe downcast and return typed pointer
			return static_cast<T*>(it->second.get());
		}

		// Resource not found - return null for safe handling by caller
		return nullptr;
	}

	template<typename T>
	bool HasResource(const std::string& pID)
	{
		// Efficient existence check without resource access overhead
		auto resourceIt = _resources.find(std::type_index(typeid(T)));

		return resourceIt != _resources.end();
	}

	template<typename T>
	void Release(const std::string& pID)
	{
		// Locate reference count entry for this resource
		auto it = _refCounts.find(std::type_index(typeid(T)));
		if (it != _refCounts.end())
		{
			it->second[pID].refCount--;

			// Check if resource has no remaining references
			if (it->second[pID].refCount <= 0)
			{
				// Step 5a: Locate and unload the unreferenced resource across all type containers
				for (auto& [type, typeResources] : _resources)
				{
					auto resourceIt = typeResources.find(pID);
					if (resourceIt != typeResources.end())
					{
						resourceIt->second->Unload();      // Allow resource to clean up its data
						typeResources.erase(resourceIt);   // Remove from cache
						break;
					}
				}

				// Step 5b: Clean up reference counting entry
				_refCounts.erase(it);
			}
		}
	}

	void UnloadAll()
	{
		// Emergency cleanup method for system shutdown or major state changes
		for (auto& [type, typeResources] : _resources)
		{
			for (auto& [id, resource] : typeResources)
			{
				resource->Unload();     // Ensure all resources clean up properly
			}

			typeResources.clear();      // Clear type-specific containers
		}

		_refCounts.clear();              // Reset all reference counts
	}
};

template<typename T>
class ResourceHandle
{
private:
	std::string _ID;
	ResourceManager* _manager = nullptr;

public:
	ResourceHandle()
	{
	}

	ResourceHandle(const std::string& pID, ResourceManager* pManager)
	{
		_ID = pID;
		_manager = pManager;
	}

	T* Get() const
	{
		if (!_manager) return nullptr;

		return _manager->GetResource<T>(_ID);
	}

	bool IsValid() const
	{
		return _manager && _manager->HasResource<T>(_ID);
	}

	const std::string& GetId() const
	{
		return _ID;
	}

	// Convenience operators
	T* operator->() const
	{
		return Get();
	}

	T& operator*() const
	{
		return *Get();
	}

	operator bool() const
	{
		return IsValid();
	}
};
