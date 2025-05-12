#pragma once
#include <string>	
#include "FGPassNode.hpp"

//template<typename T>
//struct FGResource
//{
//	struct FGResourceDescriptor
//	{
//		enum class FGResourceType
//		{
//			TEXTURE,
//			BUFFER,
//			EXTERNAL
//		} resourceType;
//		//texture
//		unsigned width, height, depth, size, mipLevels;
//		VkFormat format;
//
//		enum class Usage
//		{
//			INDEX = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
//			VERTEX = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
//			UNIFORM = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//			STORAGE = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
//		} usage;
//
//		enum class MemoryProperty
//		{
//			GPU = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//			CPU = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
//			AUTO = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, //requires cpu bit
//			LAZY = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT //not cpu visible
//		} memory;
//
//		//buffer
//	};
//
//	struct Concept
//	{
//
//	};
//
//	FGResource()
//	{
//
//	}
//	~FGResource()
//	{
//
//	}
//
//	void Create(const FGResourceDescriptor& resourceDescriptor, void* allocator)
//	{
//		static_cast<T*>(allocator)->Create(resourceDescriptor, allocator);
//	}
//
//	void Destroy(const FGResourceDescriptor& resourceDescriptor, void* allocator)
//	{
//		static_cast<T*>(allocator)->Destroy(resourceDescriptor, allocator);
//	}
//
//	void PreRead(const FGResourceDescriptor& resourceDescriptor, unsigned flags, void* context)
//	{
//
//	}
//
//	void PreWrite(const FGResourceDescriptor& resourceDescriptor, unsigned flags, void* context)
//	{
//	}
//
//	std::string ToString(const FGResourceDescriptor& resourceDescriptor)
//	{
//
//	}
//
//private:
//	VulkanContext& _vk = *VulkanContext::GetInst();
//
//	enum class Type : unsigned char
//	{
//		Transient, Imported
//	} _type;
//
//	unsigned _id;
//	unsigned _version, _initialVersion = 1;
//
//};

class FGResourceEntry
{
	struct Concept
	{
		virtual ~Concept() = default;

		virtual void Create(void* allocator) = 0;
		virtual void Destroy(void* allocator) = 0;

		virtual void PreRead(unsigned flags, void* context) = 0;
		virtual void PreWrite(unsigned flags, void* context) = 0;

		virtual std::string ToString() const = 0;
	};

	template <typename T>
	struct Model final : Concept
	{
		Model(const typename T::Desc& desc, T&& object)
		{
			descriptor = desc;
			resource = std::move(object);
		}

		void Create(void* allocator) override
		{
			resource.Create(descriptor, allocator);
		}

		void Destroy(void* allocator) override
		{
			resource.Destroy(descriptor, allocator);
		}

		void PreRead(unsigned flags, void* context) override
		{
			resource.PreRead(descriptor, flags, context);
		}

		void PreWrite(unsigned flags, void* context) override
		{
			resource.PreWrite(descriptor, flags, context);
		}

		std::string ToString() const
		{
			return "";
		}

		const typename T::Desc descriptor;
		T resource;
	};

	enum class Type
	{
		Transient, Imported
	}const _type;

	std::unique_ptr<Concept> _concept;
	const unsigned _id;
	unsigned _version;
	FGPassNode* _producer = nullptr, * _last = nullptr;

	template <typename T>
	FGResourceEntry(const Type type, unsigned id, const typename T::Desc& descriptor, T&& object)
	{
		_type = type;
		_id = id;
		_version = initialVersion;
		_concept = std::make_unique<Model<T>>(descriptor, std::forward<T>(object));
	};

public:
	FGResourceEntry() = delete;
	FGResourceEntry(const FGResourceEntry&) = delete;
	FGResourceEntry(FGResourceEntry&&) noexcept = default;

	FGResourceEntry& operator=(const FGResourceEntry&) = delete;
	FGResourceEntry& operator=(FGResourceEntry&&) noexcept = delete;

	static constexpr unsigned initialVersion = 1;

	std::string ToString() const { return _concept->ToString(); }

	void Create(void* allocator)
	{
		assert(IsTransient());

		_concept->Create(allocator);
	}

	void Destroy(void* allocator)
	{
		assert(IsTransient());

		_concept->Destroy(allocator);
	}

	void PreRead(unsigned flags, void* context)
	{
		_concept->PreRead(flags, context);
	}

	void PreWrite(unsigned flags, void* context)
	{
		_concept->PreWrite(flags, context);
	}

	unsigned GetID() const { return _id; }
	unsigned GetVersion() const { return _version; }
	bool IsImported() const { return _type == Type::Imported; }
	bool IsTransient() const { return _type == Type::Transient; }

	template <typename T>
	T& Get()
	{
		return GetModel<T>()->resource;
	}

	template <typename T>
	const typename T::Desc& GetDescriptor() const
	{
		return GetModel<T>()->descriptor;
	}

};