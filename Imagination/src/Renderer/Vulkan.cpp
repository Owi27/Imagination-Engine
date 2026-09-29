#include "pch.hpp"
#include "Vulkan.hpp"
#include "gltf/stb_image.h"
#include "EngineShaders.h"
#include "Imgn/UI/UIShader.h"

static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
{
	if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
	{
		std::cout << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
	}

	return vk::False;
}

void Vulkan::CreateDXC()
{
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
	_utils->CreateDefaultIncludeHandler(&_includeHandler);
}

void Vulkan::CreateMappedStorageBuffer_STUB() {}
