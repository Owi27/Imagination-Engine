//#include "pch.hpp"
//#include "ImgnGui.h"
//#include "ImGui/imgui.h"
//#include "ImGui/imgui_impl_win32.h"
//#include "ImGui/imgui_impl_vulkan.h"
//#include "ImgnGui.h"
//#include "ImgnGui.h"
//
//
//void ImGuiComponent::OnInit()
//{
//	Imgn::ImgnApp& app = Imgn::ImgnApp::Get();
//	IMGUI_CHECKVERSION();
//	ImGui::CreateContext();
//	ImGuiIO& io = ImGui::GetIO();
//	io.Fonts->AddFontFromFileTTF("../Fonts/Raleway-Regular.ttf", 16.f);
//	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
//	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
//	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; // Optional
//	ImGui::GetIO().MouseDrawCursor = true; // or false, depending on your needs
//	io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());
//	io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
//	ImGui::StyleColorsDark();
//
//	/*styling*/
//	auto& imgnStyle = ImGui::GetStyle();
//	imgnStyle.WindowTitleAlign = ImVec2(.5f, .5f);
//	imgnStyle.FrameRounding = 12.f;
//	imgnStyle.WindowBorderSize = 0.f;
//	imgnStyle.Colors[ImGuiCol_TitleBg] = ImVec4(0.f, 0.f, 0.f, 1.f);
//	imgnStyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.f, 0.f, 0.f, 1.f);
//	imgnStyle.Colors[ImGuiCol_TitleBg] = ImVec4(0.f, 0.f, 0.f, 1.f);
//
//	/*  Make our own style later
//	vulkanStyle = ImGui::GetStyle();
//	vulkanStyle.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
//	vulkanStyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
//	vulkanStyle.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
//	vulkanStyle.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
//	vulkanStyle.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);*/
//
//	//CreateResources();
//
//	//ImGui_ImplWin32_Init(pWin);
//	//ImGui::GetPlatformIO().Platform_CreateVkSurface = ImGui_ImplWin32_CreateVkSurface;
//
//	VkFormat format = VK_FORMAT_B8G8R8A8_SRGB;
//
//	//ImGui_ImplVulkan_InitInfo initInfo
//	//{
//	//	.ApiVersion = vk::ApiVersion14,
//	//	.Instance = *Device::Inst().GetVkInstance(),
//	//	.PhysicalDevice = *Device::Inst().GetPhysicalDevice(),
//	//	.Device = *Device::Inst().GetDevice(),
//	//	.QueueFamily = 0,
//	//	.Queue = *Device::Inst().GetQueue(),
//	//	//.DescriptorPool = *_descriptorPool,
//	//	.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE,
//	//	.MinImageCount = 3,
//	//	.ImageCount = 3,
//	//	.PipelineCache = *_pipelineCache,
//	//	.PipelineInfoMain
//	//	{
//	//		.PipelineRenderingCreateInfo =
//	//		{
//	//			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
//	//			.colorAttachmentCount = 1,
//	//			.pColorAttachmentFormats = &format,
//	//		}
//	//	},
//	//	.UseDynamicRendering = true,
//	//};
//
//	//ImGui_ImplVulkan_Init(&initInfo);
//}
//
//void ImGuiComponent::Dream(float pDeltaTime)
//{
//	ImGui_ImplVulkan_NewFrame();
//	ImGui_ImplWin32_NewFrame();
//	ImGui::NewFrame();
//	ImGuiIO& io = ImGui::GetIO();
//	io.DeltaTime = pDeltaTime;
//	
//	ImGui::SetWindowPos(ImVec2(20, 360), ImGuiCond_FirstUseEver);
//	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
//	ImGui::ShowDemoWindow();
//	ImGui::Begin("Style");
//	ImGui::ShowStyleEditor();
//	ImGui::End();
//	ImGui::Render();
//	//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *pCommandBuffer);
//
//}
//
//void ImGuiComponent::OnEvent(Event & pEvent)
//{}
//
//void ImGuiComponent::OnDestroy()
//{}
//
