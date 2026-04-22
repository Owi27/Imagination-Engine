#pragma once
#include "ImgnVulkan.hpp"

static constexpr uint32_t NumDescriptorsStreaming = 2048;
constexpr const wchar_t* VertexTarget = L"vs_6_6";
constexpr const wchar_t* FragmentTarget = L"ps_6_6";
constexpr const wchar_t* ComputeTarget = L"cs_6_6";

struct IMGN_VULKAN_API Buffer
{
	vk::raii::Buffer buffer = nullptr;
	vk::raii::DeviceMemory memory = nullptr;
};

struct IMGN_VULKAN_API Image
{
	vk::raii::Image image = nullptr;
	vk::raii::ImageView imageView = nullptr;
	vk::raii::DeviceMemory memory = nullptr;
};

class IMGN_VULKAN_API Device
{
	static inline std::unique_ptr<Device> _inst = nullptr;


	//dxc
	ComPtr<IDxcCompiler3> _compiler;
	ComPtr<IDxcUtils> _utils;
	ComPtr<IDxcIncludeHandler> _includeHandler;

public:
	vk::raii::Instance _instance;
	vk::raii::Device _device;
	vk::raii::PhysicalDevice _physicalDevice;
	vk::raii::CommandPool _commandPool;
	vk::raii::Queue _queue;

	/* Class Defaults */
	Device()
	{
		if (!_instance) _instance.reset(new Device());
	}

	~Device()
	{

	}

	/* Class Functions */
	static Device& Get() { return *_inst; }
};