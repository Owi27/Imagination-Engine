#pragma once
#include <wrl/client.h>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
using namespace Microsoft::WRL;
#include "Structs.h"

namespace ImgnVulkan
{
    inline constexpr int MAXFRAMESINFLIGHT = 3;
    inline constexpr uint32_t NumDescriptorsStreaming = 2048;

    inline constexpr const wchar_t* VertexTarget = L"vs_6_6";
    inline constexpr const wchar_t* FragmentTarget = L"ps_6_6";
    inline constexpr const wchar_t* ComputeTarget = L"cs_6_6";

    extern vk::raii::Context ctx;
    extern vk::raii::Instance instance;
    extern vk::raii::Device device;
    extern vk::raii::PhysicalDevice physicalDevice;
    extern vk::raii::CommandPool commandPool;
    extern std::vector<vk::raii::CommandBuffer> commandBuffers;
    extern vk::raii::Queue queue;

    extern uint32_t queueIdx;
    extern uint32_t frameIdx;
    extern uint32_t totalSets;

    extern vk::Extent2D swapchainExtent;
    extern vk::raii::SurfaceKHR surface;
    extern vk::raii::SwapchainKHR swapchain;
    extern vk::SurfaceFormatKHR swapchainSurfaceFormat;
    extern std::vector<vk::Image> swapchainImages;
    extern std::vector<vk::raii::ImageView> swapchainImageViews;

    extern vk::raii::DebugUtilsMessengerEXT debugMessenger;
    extern vk::raii::DescriptorPool descriptorPool;
    extern std::vector<vk::raii::DescriptorSet> descriptorSets;
    extern vk::raii::DescriptorSetLayout descriptorSetLayout;

    extern std::vector<vk::raii::Fence> inFlightFences;
    extern std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    extern std::vector<vk::raii::Semaphore> presentCompleteSemaphores;

    extern vk::raii::Sampler defaultSampler;
    extern Pipelines pipelines;

    extern ComPtr<IDxcCompiler3> compiler;
    extern ComPtr<IDxcUtils> utils;
    extern ComPtr<IDxcIncludeHandler> includeHandler;
}