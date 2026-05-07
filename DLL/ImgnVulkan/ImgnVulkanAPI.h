#pragma once
#ifdef IMGN_VULKAN_PLATFORM_WINDOWS //add an ifdef for platform
#define VK_USE_PLATFORM_WIN32_KHR 
#ifdef IMGN_VULKAN_BUILD_DLL
#define IMGN_VULKAN_API __declspec(dllexport)
#else
#define IMGN_VULKAN_API __declspec(dllimport)
#endif // IMGN_VULKAN_BUILD_DLL
#endif // IMGN_VULKAN_PLATFORM_WINDOWS
