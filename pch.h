#pragma once
#define NOMINMAX

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
#include "tinygltf/tiny_gltf.h"
#include "entt/entt.hpp"

//imgui
//#include "imgui/imgui.h"
//#include "imgui/imgui_demo.cpp"
//#include "imgui/imgui_draw.cpp"
//#include "imgui/imgui_widgets.cpp"
//#include "imgui/backends/imgui_impl_vulkan.cpp"


#include <dxcapi.h>
#include <wrl/client.h>
#pragma comment(lib, "dxcompiler.lib")

#include <filesystem>
#include <random>
#include <set>

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

#include "Structs.h"
#include "Components.h"
