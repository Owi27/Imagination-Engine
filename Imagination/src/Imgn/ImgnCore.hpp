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


template<typename T>
using unique = std::unique_ptr<T>;
template<typename T>
using Unique = std::make_unique<T>;

template<typename T>
using shared = std::shared_ptr<T>;
template<typename T>
using Shared = std::make_shared<T>;