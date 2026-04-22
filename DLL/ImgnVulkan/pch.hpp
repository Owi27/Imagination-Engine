// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.
#pragma once
#define NOMINMAX

// add headers that you want to pre-compile here
#include "framework.hpp"

#ifdef IMGN_VULKAN_PLATFORM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR //add an ifdef for platform
#ifdef IMGN_VULKAN_BUILD_DLL
#define IMGN_VULKAN_API __declspec(dllexport)
#else
#define IMGN_VULKAN_API __declspec(dllimport)
#endif // IMGN_VULKAN_BUILD_DLL
#endif // IMGN_VULKAN_PLATFORM_WINDOWS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include <map>
#include <cstdint>
#include <limits>
#include "CTX.h"
#include "Structs.h"