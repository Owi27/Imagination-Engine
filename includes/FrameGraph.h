#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <expected>

using ResourceID = unsigned;
using NodeID = unsigned;

enum class ResourceType { IMAGE, BUFFER };

struct ResourceDesc
{
	ResourceType type;
	VkFormat format;
	VkImageUsageFlags usage;
	VkExtent2D extent;
	bool transient = true;
	std::string name;
};

struct NodeDesc
{
	std::string name;
	std::vector<ResourceID> reads, writes;
	//std::function<
};

struct CompileResult
{
	std::vector<NodeDesc> plan;
	std::unordered_map<ResourceID, ResourceDesc> resources;
};

class FrameGraphBuilder
{

};

