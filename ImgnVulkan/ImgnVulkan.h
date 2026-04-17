#pragma once

#ifdef IMGNVULKAN_EXPORTS
#define IMGNVULKAN_API __declspec(dllexport)
#else
#define IMGNVULKAN_API __declspec(dllimport)
#endif

