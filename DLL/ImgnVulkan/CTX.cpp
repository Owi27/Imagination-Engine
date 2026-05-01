#include "pch.hpp"
#include "CTX.h"

namespace ImgnVulkan
{
    vk::raii::Context ctx;
    vk::raii::Instance instance = nullptr;
    vk::raii::Device device = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    vk::raii::CommandPool commandPool = nullptr;
    std::vector<vk::raii::CommandBuffer> commandBuffers;
    vk::raii::Queue queue = nullptr;

    uint32_t queueIdx = 0;
    uint32_t frameIdx = 0;
    uint32_t totalSets = MAXFRAMESINFLIGHT * 12;

    vk::Extent2D swapchainExtent{};
    vk::raii::SurfaceKHR surface = nullptr;
    vk::raii::SwapchainKHR swapchain = nullptr;
    vk::SurfaceFormatKHR swapchainSurfaceFormat{};
    std::vector<vk::Image> swapchainImages;
    std::vector<vk::raii::ImageView> swapchainImageViews;

    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    vk::raii::DescriptorPool descriptorPool = nullptr;
    std::vector<vk::raii::DescriptorSet> descriptorSets;
    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;

    std::vector<vk::raii::Fence> inFlightFences;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Semaphore> presentCompleteSemaphores;

    vk::raii::Sampler defaultSampler = nullptr;
    Pipelines pipelines;

    ComPtr<IDxcCompiler3> compiler;
    ComPtr<IDxcUtils> utils;
    ComPtr<IDxcIncludeHandler> includeHandler;
}