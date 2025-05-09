#pragma once
#include "string"

template<typename T>
struct FGResource
{
	struct FGResourceDescriptor
	{
		//texture
		unsigned width, height, depth, size, mipLevels;
		VkFormat format;
		
		enum class Usage
		{
			INDEX = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VERTEX = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			UNIFORM = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			STORAGE = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
		} usage;
		
		enum class MemoryProperty
		{
			GPU = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			CPU = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			AUTO = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, //requires cpu bit
			LAZY = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT //not cpu visible
		} memory;

		//buffer
	};

	FGResource()
	{

	}
	~FGResource()
	{

	}

	void Create(const FGResourceDescriptor& resourceDescriptor, void* allocator)
	{
		static_cast<T*>(allocator)->Create(resourceDescriptor, allocator);
	}

	void Destroy(const FGResourceDescriptor& resourceDescriptor, void* allocator)
	{
		static_cast<T*>(allocator)->Destroy(resourceDescriptor, allocator);
	}

	void PreRead(const FGResourceDescriptor& resourceDescriptor, unsigned flags, void* context)
	{

	}

	void PreWrite(const FGResourceDescriptor& resourceDescriptor, unsigned flags, void* context)
	{

	}

	std::string(const FGResourceDescriptor& resourceDescriptor)

};
