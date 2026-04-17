#pragma once

#ifdef IMGN_PLATFORM_WINDOWS
#ifdef IMGN_BUILD_DLL
#define IMGN_API __declspec(dllexport)
#else
#define IMGN_API __declspec(dllimport)
#endif // IMGN_BUILD_DLL
#endif // IMGN_PLATFORM_WINDOWS
