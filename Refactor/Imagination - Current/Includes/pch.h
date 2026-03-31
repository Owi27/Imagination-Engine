#pragma once

#define NOMINMAX
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <memory>
#include <optional>
#include <iostream>
#include <cstdint>
#include <limits>
#include <chrono>
#include <typeindex>
#include <functional>
#include <queue>
#include <mutex>

#define VK_USE_PLATFORM_WIN32_KHR //add an ifdef for platform
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include "Events/Event.h"

#include "Device.h"

#include "ImgnMath.h"

#include "Component.h"
#include "ResourceManager.h"
