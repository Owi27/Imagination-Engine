#pragma once
#include "ImGui/imgui.h"
#include "ImGui//imgui_impl_win32.h"
#include "ImGui/imgui_impl_vulkan.h"

class ImGuiContext
{
	VulkanContext& _vk;
	VkCommandBuffer _commandBuffer;
	VkDescriptorPool _descriptorPool;
	VkDescriptorSetLayout _descriptorSetLayout;
	VkDescriptorSet _descriptorSet;

public:
	ImGuiContext() : _vk(*VulkanContext::GetInst())
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplWin32_Init(_vk.GetWindowHandle());
		ImGui_ImplVulkan_InitInfo initInfo
		{
			.Instance = _vk.GetInstance(),
			.PhysicalDevice = _vk.GetPhysicalDevice(),
			.Device = _vk.GetDevice(),
			.Queue = _vk.GetGraphicsQueue(),
			.DescriptorPool = _descriptorPool,
			.UseDynamicRendering = true
		};

		ImGui_ImplVulkan_Init(&initInfo);

		GvkHelper::signal_command_start(_vk.GetDevice(), _vk.GetCommandPool(), &_commandBuffer);

		//ImGui_ImplVulkan_CreateFontsTexture(_commandBuffer);
		GvkHelper::signal_command_end(_vk.GetDevice(), _vk.GetGraphicsQueue(), _vk.GetCommandPool(), &_commandBuffer);
	}

	~ImGuiContext()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void Render();
};

