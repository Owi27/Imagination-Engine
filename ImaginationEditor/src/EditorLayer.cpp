#include "EditorLayer.hpp"

#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_vulkan.h"

namespace Imgn
{
	void EditorLayer::UpdateCamera(Time pTime)
	{
		if (!Input::IsKeyPressed(IMGN_MOUSE_BUTTON_RIGHT)) return;
		float speed = _cameraSpeed * pTime;
		float moveX = 0.f;
		float moveZ = 0.f;

		if (Input::IsKeyPressed(IMGN_KEY_W)) moveZ += 1.f;
		else if (Input::IsKeyPressed(IMGN_KEY_S)) moveZ -= 1.f;
		if (Input::IsKeyPressed(IMGN_KEY_D)) moveX += 1.f;
		else if (Input::IsKeyPressed(IMGN_KEY_A)) moveX -= 1.f;

		// local movement -> world movement
		_camPos[0] += (moveX * std::cos(_yaw) + moveZ * std::sin(_yaw)) * speed;
		_camPos[2] += (-moveX * std::sin(_yaw) + moveZ * std::cos(_yaw)) * speed;

		if (Input::IsKeyPressed(IMGN_KEY_SPACE))
		{
			if (Input::IsKeyPressed(IMGN_KEY_LEFT_SHIFT)) _camPos[1] -= _cameraSpeed * pTime;
			else _camPos[1] += _cameraSpeed * pTime;
		}

		// local movement -> world movement
		_camPos[0] += (moveX * std::cos(_yaw) + moveZ * std::sin(_yaw)) * pTime;
		_camPos[2] += (-moveX * std::sin(_yaw) + moveZ * std::cos(_yaw)) * pTime;

		_camera.SetPosition(_camPos);

		//rotation
		auto [deltaX, deltaY] = Input::GetMouseDelta();
		float pitch = Math::Radians(45.f) * deltaY / _window->GetHeight();
		float yaw = Math::Radians(45.f) * _window->GetAspectRatio() * deltaX / _window->GetWidth();

		_yaw += yaw;

		_camera.Rotate(pitch, yaw);
	}

	void EditorLayer::Sleep()
	{
	}

	void EditorLayer::WakeUp()
	{
	}

	void EditorLayer::OnImGuiRender()
	{
		ImGui::DockSpaceOverViewport();

		// Show demo options and help
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit")) ImgnApp::Get().Close();
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		ImGui::ShowDemoWindow();
		ImGui::Begin("Style");
		ImGui::ShowStyleEditor();
		ImGui::End();

		if (!_sceneWindow)
		{
			_sceneWindow = static_cast<vk::DescriptorSet>(ImGui_ImplVulkan_AddTexture(*_renderer->GetTextureSampler(), **_renderer->GetRenderGraphImage("LitScene").image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
		}

		ImTextureID textureID = static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(static_cast<VkDescriptorSet>(_sceneWindow)));

		ImGui::Begin("Vulkan Texture Test");
		ImGui::Image(ImTextureRef(textureID), ImVec2(_window->GetWidth(), _window->GetHeight()));
		ImGui::End();
	}

	void EditorLayer::Dream(Time pTime)
	{
		UpdateCamera(pTime);

		GBufferUBO gBufferUBO
		{
			.viewProj = _camera.GetViewProj()
		};
		_renderer->MapBufferData(gBufferUBOHandle, &gBufferUBO, sizeof(GBufferUBO));
		//IMGN_INFO("DeltaTime {}s : {}ms", pTime.Seconds(), pTime.MiliSeconds());

		if (Input::IsKeyPressed(IMGN_KEY_TAB))
		{
			IMGN_INFO("Tab key pressed");
		}

		_renderer->ExecuteGraph();
		_renderer->BlitToSwapchain("LitScene");
	}

	void EditorLayer::OnEvent(Event& pEvent)
	{
	}
}