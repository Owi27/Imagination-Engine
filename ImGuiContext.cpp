#include "pch.h"
#include "ImGuiContext.h"

void ImGuiContext::Render()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _commandBuffer);
}
