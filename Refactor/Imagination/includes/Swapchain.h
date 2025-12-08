#pragma once
#include "VkResources.h"

class Swapchain
{
	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	//std::unique_ptr<Texture> _depth;

	void CreateSwapchain();
	void CreateSwapchainImageViews();
	void CreateCommandBuffers();
	void CreateDepthTexture();

	struct QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice pPhysicalDevice);
	VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& pAvailableFormats);
	VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& pAvailablePresentModes);
	VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& pSurfaceCapabilities);

public:
	VkSwapchainKHR swapchain;
	VkExtent3D swapchainExtent;
	VkFormat swapchainFormat;
	std::vector<std::unique_ptr<Texture>> swapchainImages;
	std::vector<VkCommandBuffer> swapchainCommandBuffers;

	Swapchain() /*Constructor*/
	{
	}

	~Swapchain() /*Destructor*/
	{
	}

	Swapchain(const Swapchain& pOther) = delete;

	Swapchain& operator=(const Swapchain& pOther) = delete;

	Swapchain(Swapchain&& pOther) noexcept /*Move Constructor*/
	{
	}

	Swapchain& operator=(Swapchain&& pOther) noexcept /*Move Assignment Operator*/
	{
		if (this != &pOther)
		{
		}

		return *this;
	}

	void Init();
	SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice pPhysicalDevice, VkSurfaceKHR pSurface);
};