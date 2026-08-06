#pragma once

#ifdef IMGN_PLATFORM_WINDOWS
#ifdef IMGN_BUILD_DLL
#define IMGN_API __declspec(dllexport)
#else
#define IMGN_API __declspec(dllimport)
#endif // IMGN_BUILD_DLL
#endif // IMGN_PLATFORM_WINDOWS

#ifdef IMGN_ENABLE_ASSERTS
#define IMGN_ASSERT(x, ...) {if(!(x)) {IMGN_ERROR("Assertion Failed: {}", __VA_ARGS__); __debugbreak();}}
#define IMGN_CORE_ASSERT(x, ...) {if(!(x)) {IMGN_CORE_ERROR("Assertion Failed: {}", __VA_ARGS__); __debugbreak();}}
#else
#define IMGN_ASSERT(x, ...)
#define IMGN_CORE_ASSERT(x, ...)
#endif // IMGN_ENABLE_ASSERTS

#define BIT(x) (1 << x)

//#include <memory>

template<typename T>
using unique = std::unique_ptr<T>;
template<typename T, typename... Args>
unique<T> Unique(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using shared = std::shared_ptr<T>;
template<typename T, typename... Args>
shared<T> Shared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

//types
using vec2 = std::array<float, 2>;
using vec3 = std::array<float, 3>;
using vec4 = std::array<float, 4>;
using mat3 = std::array<float, 9>;
using mat4 = std::array<float, 16>;
