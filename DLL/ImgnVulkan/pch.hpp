// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.
#pragma once
#define NOMINMAX

// add headers that you want to pre-compile here
#include "framework.hpp"

#ifdef IMGN_VULKAN_PLATFORM_WINDOWS //add an ifdef for platform
#define VK_USE_PLATFORM_WIN32_KHR 
#endif // IMGN_VULKAN_PLATFORM_WINDOWS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <vector>
#include <array>
#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include <map>
#include <cstdint>
#include <limits>
#include <algorithm>

#pragma once
#include <wrl/client.h>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
using namespace Microsoft::WRL;