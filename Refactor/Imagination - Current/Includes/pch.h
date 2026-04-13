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

//gateware enables
#define GATEWARE_ENABLE_CORE // All libraries need this
#define GATEWARE_ENABLE_SYSTEM // Graphics libs require system level libraries
#define GATEWARE_ENABLE_GRAPHICS // Enables all Graphics Libraries
#define GATEWARE_ENABLE_INPUT //Enable all input libraries
#define GATEWARE_ENABLE_MATH //enable all math libraries
#define GATEWARE_ENABLE_MATH2D

//TO implement later
#define GATEWARE_DISABLE_GDIRECTX11SURFACE // we have another template for this
//#define GATEWARE_DISABLE_GDIRECTX12SURFACE // we have another template for this
#define GATEWARE_DISABLE_GRASTERSURFACE // we have another template for this
#define GATEWARE_DISABLE_GOPENGLSURFACE // we have another template for this
// With what we want & what we don't defined we can include the API
#include "Gateware/Gateware.h"
using GWindow = GW::SYSTEM::GWindow;
using GWindowStyle = GW::SYSTEM::GWindowStyle;
using GVulkanSurface = GW::GRAPHICS::GVulkanSurface;
using GDirectX12Surface = GW::GRAPHICS::GDirectX12Surface;
using GEventReceiver = GW::CORE::GEventReceiver;
using GEventResponder = GW::CORE::GEventResponder;

using GInput = GW::INPUT::GInput;
using GController = GW::INPUT::GController;
using GMatrix = GW::MATH::GMatrix;
using mat4 = GW::MATH::GMATRIXF;
using vec4 = GW::MATH::GVECTORF;
using GVector2D = GW::MATH2D::GVector2D;
using mat3 = GW::MATH2D::GMATRIX3F;
using vec3 = GW::MATH2D::GVECTOR3F;
using vec2 = GW::MATH2D::GVECTOR2F;
using GEvent = GW::GEvent;
using GBufferedInput = GW::INPUT::GBufferedInput;

#include "Events/Event.h"

#include "Input.h"
#include "ImgnKeyMouseCodes.h"

#include "Device.h"
#include "Structs.h"

//#include "ImgnMath.h"

#include "Component.h"
#include "ResourceManager.h"
