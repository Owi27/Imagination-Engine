#pragma once
#include "ImgnRenderResources.h"
#include <ImgnVulkan.hpp>

struct ImgnRenderPass
{
	std::string name;

	std::vector<std::string> textureInputs, textureOutputs, bufferInputs, bufferOutputs;

	std::function<void(vk::raii::CommandBuffer&)> Execute;
};

struct ImgnImage
{
	std::string name;

	ImgnFormat format;
	ImgnExtent2D extent;
	ImgnImageUsage usage;
	//ImgnImageLayout initialLayout, currentLayout, finalLayout;
	//ImgnAspect aspect;

	//vulkan
	ImgnVulkan::Image vkImage;
};

struct ImgnBuffer
{
	std::string name;

	uint64_t size;
	ImgnBufferUsage usage;

	ImgnVulkan::Buffer vkBuffer;
};

class ImgnRenderGraph
{
	std::unordered_map<std::string, ImgnImage> _imageResources;
	std::unordered_map<std::string, ImgnBuffer> _bufferResources;

	std::vector<ImgnRenderPass> _renderPasses;
	std::vector<uint32_t> _executionOrder;

public:
	/* Class Defaults */
	ImgnRenderGraph()
	{

	}

	~ImgnRenderGraph()
	{

	}

	/* Class Functions */
};