#include "pch.hpp"
#include "SceneHierarchy.h"

#include <ImGui/imgui.h>
#include <Imgn/Components/ImgnComponents.h>

namespace Imgn
{
	void SceneHierarchy::DrawComponents(Entity* pEntity)
	{
		char buffer[32];
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, sizeof(buffer), pEntity->GetName().c_str());
		if (ImGui::InputText("Name", buffer, sizeof(buffer)))
		{
			pEntity->SetName(std::string(buffer));
		}

		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap;

		if (TransformComponent* transform = pEntity->GetComponent<TransformComponent>())
		{
			CreateComponentSettings<TransformComponent>(pEntity, "Transform", [&](bool pOpen)
				{
					if (pOpen)
					{
						ImGui::DragFloat3("Position", transform->position.data(), 0.1f);
						ImGui::DragFloat3("Rotation", transform->rotation.data(), 0.5f);
						ImGui::DragFloat3("Scale", transform->scale.data(), 0.01f);
					}
				});
		}

		if (CameraComponent* camera = pEntity->GetComponent<CameraComponent>())
		{
			CreateComponentSettings<CameraComponent>(pEntity, "Camera", [&](bool pOpen)
				{
					if (pOpen)
					{
						std::array projectionTypes = { "Perspective", "Orthographic" };

						if (ImGui::BeginCombo("Projection", projectionTypes[static_cast<int>(camera->camera.GetType())]))
						{
							for (size_t i = 0; i < projectionTypes.size(); i++)
							{
								bool selected = projectionTypes[static_cast<int>(camera->camera.GetType())] == projectionTypes[i];

								if (ImGui::Selectable(projectionTypes[i], selected))
								{
									camera->camera.SetType(static_cast<CameraType>(i));
								}

								if (selected) ImGui::SetItemDefaultFocus();
							}

							ImGui::EndCombo();
						}

						ImGui::DragFloat("Cam Speed", &camera->cameraSpeed, 0.1f);

						if (camera->camera.GetType() == CameraType::Perspective)
						{
							float fov = camera->camera.GetFOV(), nearPlane = camera->camera.GetNearPlane(), farPlane = camera->camera.GetFarPlane();

							if (ImGui::DragFloat("FOV", &fov, 0.1f) || ImGui::DragFloat("Near", &nearPlane, 0.1f) || ImGui::DragFloat("Far", &farPlane, 0.1f))
							{
								camera->camera.CreatePerspectiveCamera(fov, nearPlane, farPlane);
							}
						}

						if (camera->camera.GetType() == CameraType::Orthographic)
						{
							float orthoSize = camera->camera.GetOrthoSize(), orthoNear = camera->camera.GetNearPlane(), orthoFar = camera->camera.GetFarPlane();

							if (ImGui::DragFloat("Ortho Size", &orthoSize, 0.1f) || ImGui::DragFloat("Near", &orthoNear, 0.1f) || ImGui::DragFloat("Far", &orthoFar, 0.1f))
							{
								camera->camera.CreateOrthographicCamera(orthoSize, orthoNear, orthoFar);
							}
						}
					}
				});
		}
	}

	template<typename T>
	void SceneHierarchy::CreateComponentSettings(Entity* pEntity, std::string_view pComponentTitle, std::function<void(bool)> pComponentSettings)
	{
		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap;

		bool open = ImGui::CollapsingHeader(pComponentTitle.data(), flags);
		ImGui::SameLine();
		if (ImGui::Button("+"))
		{
			ImGui::OpenPopup("Component Settings");
		}

		bool removeComponent = false;
		if (ImGui::BeginPopup("Component Settings"))
		{
			if (ImGui::MenuItem("Remove Component"))
			{
				removeComponent = true;
			}

			ImGui::EndPopup();
		}

		pComponentSettings(open);

		if (removeComponent) pEntity->RemoveComponent<T>();
	}

	void SceneHierarchy::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		for (auto& entity : _scene->GetEntities())
		{
			ImGuiTreeNodeFlags flags = ((_selectedEntity == entity.get()) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
			bool opened = ImGui::TreeNodeEx(entity->GetName().c_str(), flags);

			if (ImGui::IsItemClicked())
			{
				_selectedEntity = entity.get();
			}

			bool entityDeleted = false;
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Delete Entity"))
				{
					entityDeleted = true;
				}

				ImGui::EndPopup();
			}

			if (opened)
			{
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
				bool opened = ImGui::TreeNodeEx(std::format("{}{}", entity->GetName(), "777").c_str(), flags);
				if (opened) ImGui::TreePop();
				ImGui::TreePop();
			}

			if (entityDeleted)
			{
				if (_selectedEntity == entity.get()) _selectedEntity = nullptr;

				_scene->DestroyEntity(entity);
			}
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			_selectedEntity = nullptr;
		}

		if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				_scene->CreateEntity("Empty Entity");
			}

			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Inspector");

		if (_selectedEntity)
		{
			DrawComponents(_selectedEntity);

			if (ImGui::Button("Add Component"))
			{
				ImGui::OpenPopup("AddComponent");
			}

			if (ImGui::BeginPopup("AddComponent"))
			{
				if (ImGui::MenuItem("Camera"))
				{
					_selectedEntity->AddComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}


				ImGui::EndPopup();
			}
		}

		ImGui::End();
	}
}