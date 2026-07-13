#pragma once
#include "ImgnRenderResources.h"
#include <ImgnVulkan.hpp>

struct ImgnRenderPass
{
	std::string name;

	std::vector<std::string> textureInputs, textureOutputs, bufferInputs, bufferOutputs;

	std::function<void(ImgnCommandBuffer&)> Execute;
};

struct ImgnImage
{
	std::string name;
	uint32_t handle;

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
	
	ImgnCommandBuffer RecordSingleCommand();
	void EndSingleCommand(ImgnCommandBuffer& pCommandBuffer);

	void CreateImageResource(ImgnImageDesc& pImage);
	void CreateBufferResource(ImgnBuffer& pBuffer);

public:
	/* Class Defaults */
	ImgnRenderGraph()
	{

	}

	~ImgnRenderGraph()
	{

	}

	/* Class Functions */
	uint32_t AddResource(const std::string& pName, ImgnFormat pFormat, ImgnExtent3D pExtent, ImgnImageUsage pUsage, ImgnImageLayout pInitialLayout, ImgnImageLayout pFinalLayout, ImgnAspect pAspect);
	void AddResource(const std::string& pName, vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, const void* pData);
	void AddPass(ImgnRenderPass pRenderPass) { _renderPasses.emplace_back(std::move(pRenderPass)); }

};