#pragma once
#include <string>

enum class FGResourceType
{
	TEXTURE,
	BUFFER,
	EXTERNAL
};

enum class FGResourceUsageType
{
	READ,
	WRITE,
	CREATE_WRITE,
	READ_WRITE
};

struct FGTextureCreateInfo
{
	std::string name;
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	VkExtent3D extent = { 0, 0, 1 };
	unsigned mipLevels = 1, arrayLayers = 1;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL;
	VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkClearColorValue clearColor = { {0.0f, 0.0f, 0.0f, 1.0f} };
	VkClearDepthStencilValue clearDepthStencil = { 1.0f, 0 };

	static FGTextureCreateInfo AsColorAttachment(const std::string& name, VkExtent2D extent, VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	static FGTextureCreateInfo AsDepthAttachment(const std::string& name, VkExtent2D extent, VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
};

struct FGBufferCreateInfo
{
	std::string name;
	VkDeviceSize size = 0;
	VkBufferUsageFlags usage = 0;
	VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
};

class FGResourceHandle 
{
	unsigned _id;
	unsigned _version; // Tracks different states of a resource if it's written multiple times

public:
	FGResourceHandle() : _id(UINT32_MAX), _version(0) // Invalid handle
	{
	}
	explicit FGResourceHandle(unsigned id, unsigned version = 0) : _id(id), _version(version) 
	{
	}

	bool IsValid() const { return _id != UINT32_MAX; }
	unsigned GetId() const { return _id; }
	unsigned GetVersion() const { return _version; } // For future use with resource versioning

	bool operator==(const FGResourceHandle& other) const { return _id == other._id && _version == other._version; }
	bool operator!=(const FGResourceHandle& other) const { return !(*this == other); }
	bool operator<(const FGResourceHandle& other) const 
	{ // For use in maps/sets
		if (_id != other._id) return _id < other._id;
		return _version < other._version;
	}
	
	struct Hasher {
		std::size_t operator()(const FGResourceHandle& h) const 
		{
			// Simple hash, can be improved
			return std::hash<unsigned>()(h.GetId()) ^ (std::hash<unsigned>()(h.GetVersion()) << 1);
		}
	};
};

// Structure to define how a pass uses a resource
struct FGResourceUsage 
{
	FGResourceHandle handle;
	FGResourceUsageType type;
	VkImageLayout expectedLayout = VK_IMAGE_LAYOUT_UNDEFINED; // For textures
	VkAccessFlags accessFlags = 0;          // Access flags for this usage
	VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // Pipeline stage for this usage

	// Optional creation info if this usage creates the resource
	std::variant<std::monostate, FGTextureCreateInfo, FGBufferCreateInfo> creationInfo;

	FGResourceUsage(FGResourceHandle h, FGResourceUsageType t) : handle(h), type(t) {}
};

// Special name for the final swapchain image resource
static const std::string SWAPCHAIN_RESOURCE_NAME = "Backbuffer_Swapchain_Image";

// Queue types for passes (simplified for now)
enum class FGPassQueueType {
	GRAPHICS,
	COMPUTE,
	TRANSFER
	// ASYNC_COMPUTE etc. could be added
};