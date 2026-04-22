#pragma once
#include <wrl/client.h>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
using namespace Microsoft::WRL;

namespace ImgnVulkan
{
	constexpr int MAXFRAMESINFLIGHT = 3;
	static constexpr uint32_t NumDescriptorsStreaming = 2048;
	constexpr const wchar_t* VertexTarget = L"vs_6_6";
	constexpr const wchar_t* FragmentTarget = L"ps_6_6";
	constexpr const wchar_t* ComputeTarget = L"cs_6_6";

	vk::raii::Context ctx;
	vk::raii::Instance instance = nullptr;
	vk::raii::Device device = nullptr;
	vk::raii::PhysicalDevice physicalDevice = nullptr;
	vk::raii::CommandPool commandPool = nullptr;
	std::vector<vk::raii::CommandBuffer> commandBuffers;
	vk::raii::Queue queue = nullptr;
	uint32_t queueIdx = 0, frameIdx = 0;
	vk::Extent2D swapchainExtent;
	vk::raii::SurfaceKHR surface = nullptr;
	vk::raii::SwapchainKHR swapchain = nullptr;
	vk::SurfaceFormatKHR swapchainSurfaceFormat;
	std::vector<vk::Image> swapchainImages;
	std::vector<vk::raii::ImageView> swapchainImageViews;
	vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
	vk::raii::DescriptorPool descriptorPool = nullptr;
	std::vector<vk::raii::DescriptorSet> descriptorSets;
	vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
	uint32_t totalSets = MAXFRAMESINFLIGHT * 12;
	std::vector<vk::raii::Fence> inFlightFences;
	std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
	std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
	vk::raii::Sampler defaultSampler = nullptr;

	Pipelines pipelines;

	//dxc
	ComPtr<IDxcCompiler3> compiler;
	ComPtr<IDxcUtils> utils;
	ComPtr<IDxcIncludeHandler> includeHandler;
}