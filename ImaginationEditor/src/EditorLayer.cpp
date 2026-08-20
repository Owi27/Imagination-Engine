#include "EditorLayer.hpp"

#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_vulkan.h"

namespace Imgn
{
	void EditorLayer::UpdateCamera(Time pTime)
	{
		auto* transform = _mainCamera->GetComponent<TransformComponent>();

		auto* cameraComponent = _mainCamera->GetComponent<CameraComponent>();

		if (!transform || !cameraComponent) return;

		auto [deltaX, deltaY] = Input::GetMouseDelta();

		if (!Input::IsKeyPressed(IMGN_MOUSE_BUTTON_RIGHT)) return;

		vec3& position = transform->position;
		vec3& rotation = transform->rotation;
		const float speed = cameraComponent->cameraSpeed * pTime;

		const float pitchDelta = 45.f * deltaY / static_cast<float>(_window->GetHeight());
		const float yawDelta = 45.f * _window->GetAspectRatio() * deltaX / static_cast<float>(_window->GetWidth());

		rotation[0] += pitchDelta;
		rotation[1] += yawDelta;

		//// Prevent flipping over vertically.
		//rotation[0] = std::clamp(rotation[0], -89.f, 89.f);

		//movement
		float moveX = 0.f;
		float moveZ = 0.f;

		if (Input::IsKeyPressed(IMGN_KEY_W)) moveZ += 1.f;
		else if (Input::IsKeyPressed(IMGN_KEY_S)) moveZ -= 1.f;
		if (Input::IsKeyPressed(IMGN_KEY_D)) moveX += 1.f;
		else if (Input::IsKeyPressed(IMGN_KEY_A)) moveX -= 1.f;

		// Prevent diagonal movement from being faster.
		/*if (moveX != 0.f && moveZ != 0.f)
		{
			constexpr float diagonal = 0.70710678f;

			moveX *= diagonal;
			moveZ *= diagonal;
		}*/

		const float yaw = Math::Radians(rotation[1]);
		position[0] += (moveX * std::cos(yaw) + moveZ * std::sin(yaw)) * speed;
		position[2] += (-moveX * std::sin(yaw) + moveZ * std::cos(yaw)) * speed;

		if (Input::IsKeyPressed(IMGN_KEY_SPACE))
		{
			if (Input::IsKeyPressed(IMGN_KEY_LEFT_SHIFT)) position[1] -= speed;
			else position[1] += speed;
		}



		//vec3& camPos = _mainCamera->GetComponent<TransformComponent>()->position;

		//float speed = _mainCamera->GetComponent<CameraComponent>()->cameraSpeed * pTime;
		//float moveX = 0.f;
		//float moveZ = 0.f;

		//if (Input::IsKeyPressed(IMGN_KEY_W)) moveZ += 1.f;
		//else if (Input::IsKeyPressed(IMGN_KEY_S)) moveZ -= 1.f;
		//if (Input::IsKeyPressed(IMGN_KEY_D)) moveX += 1.f;
		//else if (Input::IsKeyPressed(IMGN_KEY_A)) moveX -= 1.f;

		//// local movement -> world movement
		//camPos[0] += (moveX * std::cos(_yaw) + moveZ * std::sin(_yaw)) * speed;
		//camPos[2] += (-moveX * std::sin(_yaw) + moveZ * std::cos(_yaw)) * speed;


		//if (Input::IsKeyPressed(IMGN_KEY_SPACE))
		//{
		//	if (Input::IsKeyPressed(IMGN_KEY_LEFT_SHIFT)) camPos[1] -= speed;
		//	else camPos[1] += speed;
		//}

		//_mainCamera->GetComponent<TransformComponent>()->position = camPos;

		////rotation
		//float pitch = Math::Radians(45.f) * deltaY / _window->GetHeight();
		//float yaw = Math::Radians(45.f) * _window->GetAspectRatio() * deltaX / _window->GetWidth();

		//_yaw += yaw;

		//_mainCamera->GetComponent<TransformComponent>()->rotation[0] = pitch;
		//_mainCamera->GetComponent<TransformComponent>()->rotation[1] = yaw;
		////_camera.Rotate(pitch, yaw);
	}

	void EditorLayer::Sleep()
	{
		ImgnGLTF gltf;
		ImgnModel sponza = gltf.LoadModel("../../Models/Sponza/glTF/Sponza.gltf", *_renderer);

		RenderPass gBuffer
		{
			.name = "G-BufferPass",
			.imageOUT =
			{
				_renderer->CreateRGImageDesc("G-BufferAlbedo", _window->GetWidth(), _window->GetHeight(), vk::Format::eR8G8B8A8Srgb),
				_renderer->CreateRGImageDesc("G-BufferNormal", _window->GetWidth(), _window->GetHeight(), vk::Format::eR8G8B8A8Unorm),
				_renderer->CreateRGImageDesc("G-BufferMaterial", _window->GetWidth(), _window->GetHeight(), vk::Format::eR8G8B8A8Unorm),
				_renderer->CreateRGImageDesc("G-BufferEmissive", _window->GetWidth(), _window->GetHeight(), vk::Format::eR8G8B8A8Srgb),
				_renderer->CreateRGImageDesc("Depth", _window->GetWidth(), _window->GetHeight(), vk::Format::eD32Sfloat)
			},
			.Execute = [&, sponza](Imgn::RenderContext& ctx)
			{
				std::vector colorAttachments =
				{
					ctx.CreateRenderingAttachmentInfo("G-BufferAlbedo"),
					ctx.CreateRenderingAttachmentInfo("G-BufferNormal"),
					ctx.CreateRenderingAttachmentInfo("G-BufferMaterial"),
					ctx.CreateRenderingAttachmentInfo("G-BufferEmissive"),
				};

				vk::RenderingAttachmentInfo depthAttachment = ctx.CreateRenderingAttachmentInfo("Depth");

				ctx.BeginRendering(_window->GetWidth(), _window->GetHeight(), colorAttachments, &depthAttachment);
				ctx.BindPipeline(vk::PipelineBindPoint::eGraphics, _renderer->GetGBufferPipeline());
				ctx.BindDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), 1, *_renderer->GetTextureDescriptorSet());
				ctx.SetViewport(_window->GetWidth(), _window->GetHeight());
				ctx.SetScissor(_window->GetWidth(), _window->GetHeight());

				vk::DescriptorBufferInfo uboInfo = ctx.CreateDescriptorBufferInfo(gBufferUBOHandle, sizeof(GBufferUBO));
				vk::DescriptorBufferInfo materialSBInfo = ctx.CreateDescriptorBufferInfo(sponza.materialBuffer, sponza.materialBufferSize);

				std::vector writes
				{
					ctx.CreateWriteDescriptorSet(0, vk::DescriptorType::eUniformBuffer, uboInfo),
					ctx.CreateWriteDescriptorSet(1, vk::DescriptorType::eStorageBuffer, materialSBInfo),
				};

				ctx.PushDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), writes);

				for (auto& entity : _activeScene->GetEntities())
				{
					if (!entity->IsActive()) continue;

					if (TransformComponent* transform = entity->GetComponent<TransformComponent>())
					{
						if (MeshComponent* meshComp = entity->GetComponent<MeshComponent>())
						{
							ctx.BindMesh(meshComp->mesh);

							for (ImgnPrimitive& prim : _renderer->GetMesh(meshComp->mesh).primitives)
							{
								GBufferPC pc
								{
									.model = transform->GetTransform(),
									.materialIndex = prim.material
								};

								ctx.PushConstants<GBufferPC>(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, pc);
								ctx.DrawPrimitive(prim);
							}
						}
					}
				}

				ctx.EndRendering();
			}
		};

		RenderPass lighting
		{
			.name = "LightingPass",
			.imageIN =
			{
				"G-BufferAlbedo",
				"G-BufferNormal",
				"G-BufferMaterial",
				"G-BufferEmissive",
				"Depth"
			},
			.imageOUT =
			{
				_renderer->CreateRGImageDesc("LitScene", _window->GetWidth(), _window->GetHeight(), vk::Format::eR16G16B16A16Sfloat)
			},
			.Execute = [&](Imgn::RenderContext& ctx)
			{
				/*vk::RenderingAttachmentInfo colorAttachment
				{
					.imageView = *Renderer().GetRenderGraphImage(Renderer().MakeImageKey("LitScene", GetWindow().GetWidth(), GetWindow().GetHeight())).image.view,
					.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
					.loadOp = vk::AttachmentLoadOp::eClear,
					.storeOp = vk::AttachmentStoreOp::eStore,
					.clearValue = vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} },
				};

				vk::RenderingAttachmentInfo depthAttachment
				{
					.imageView = *Renderer().GetRenderGraphImage(Renderer().MakeImageKey("Depth", GetWindow().GetWidth(), GetWindow().GetHeight())).image.view,
					.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
					.loadOp = vk::AttachmentLoadOp::eClear,
					.storeOp = vk::AttachmentStoreOp::eStore,
					.clearValue = vk::ClearDepthStencilValue{ 1.0f, 0 },
				};

				vk::RenderingInfo renderingInfo
				{
					.renderArea = { {0, 0}, { GetWindow().GetWidth(), GetWindow().GetHeight() } },
					.layerCount = 1,
					.colorAttachmentCount = 1,
					.pColorAttachments = &colorAttachment,
					.pDepthAttachment = &depthAttachment,
				}; */

				ctx.BindPipeline(vk::PipelineBindPoint::eCompute, _renderer->GetLightingPipeline());
				ctx.BindDescriptorSet(vk::PipelineBindPoint::eCompute, _renderer->GetPipelineLayout(), 1, *_renderer->GetTextureDescriptorSet());

				LightingPC pc
				{
					.invViewProj = Math::Inverse(_mainCamera->GetComponent<TransformComponent>()->GetTransform() * _mainCamera->GetComponent<CameraComponent>()->camera.GetProjection()),
					.camPos = _mainCamera->GetComponent<TransformComponent>()->position,
					.width = _window->GetWidth(),
					.height = _window->GetHeight()
				};

				ctx.PushConstants<LightingPC>(vk::ShaderStageFlagBits::eCompute, pc);

				//push descriptor set
				{
					//uniform buffer
					/*vk::DescriptorBufferInfo uboInfo
					{
						.buffer = *Renderer().GetRenderGraphBuffer("G-BufferUBO").buffer.buffer,
						.offset = 0,
						.range = 192
					};*/

					std::array images =
					{
						ctx.CreateDescriptorImageInfo("G-BufferAlbedo"),
						ctx.CreateDescriptorImageInfo("G-BufferNormal"),
						ctx.CreateDescriptorImageInfo("G-BufferMaterial"),
						ctx.CreateDescriptorImageInfo("G-BufferEmissive"),
						ctx.CreateDescriptorImageInfo("Depth"),
					};

					vk::DescriptorImageInfo litImage = ctx.CreateDescriptorImageInfo("LitScene", vk::ImageLayout::eGeneral);


					std::array writes
					{
						ctx.CreateWriteDescriptorSet(2, vk::DescriptorType::eSampledImage, images),
						ctx.CreateWriteDescriptorSet(3, vk::DescriptorType::eStorageImage, litImage)
					};

					ctx.PushDescriptorSet(vk::PipelineBindPoint::eCompute, _renderer->GetPipelineLayout(), writes);
				}

				ctx.Dispatch(std::ceil(_window->GetWidth() / 8), std::ceil(_window->GetHeight() / 8), 1);
			}
		};

		_renderer->AddPass(gBuffer);
		_renderer->AddPass(lighting);
		_renderer->CompileGraph();

		_activeScene = Shared<Scene>();

		for (auto& meshHandle : sponza.meshes)
		{
			Entity* entity = _activeScene->CreateEntity("Sponza");
			entity->AddComponent<Imgn::MeshComponent>(meshHandle);
		}

		_mainCamera = _activeScene->CreateEntity("SceneCamera");
		CameraComponent* camera = _mainCamera->AddComponent<CameraComponent>();
		TransformComponent* cameraTransform = _mainCamera->GetComponent<TransformComponent>();
		camera->camera.SetViewportSize(_window->GetWidth(), _window->GetHeight());


		class TestScript : public ScriptableEntity
		{
			float _yaw = 0.f;
			CameraComponent* cam;// = GetComponent<CameraComponent>();
			TransformComponent* transform;// = GetComponent<TransformComponent>();

			float mouseSensitivity = 1.f;

			void UpdateCamera(Time pTime)
			{
				auto [deltaX, deltaY] = Input::GetMouseDelta();

				if (!Input::IsKeyPressed(IMGN_MOUSE_BUTTON_RIGHT)) return;
				float speed = cam->cameraSpeed * pTime;
				float moveX = 0.f;
				float moveZ = 0.f;
				vec3 camPos = transform->position;

				if (Input::IsKeyPressed(IMGN_KEY_W)) moveZ += 1.f;
				else if (Input::IsKeyPressed(IMGN_KEY_S)) moveZ -= 1.f;
				if (Input::IsKeyPressed(IMGN_KEY_D)) moveX += 1.f;
				else if (Input::IsKeyPressed(IMGN_KEY_A)) moveX -= 1.f;

				// local movement -> world movement
				camPos[0] += (moveX * std::cos(_yaw) + moveZ * std::sin(_yaw)) * speed;
				camPos[2] += (-moveX * std::sin(_yaw) + moveZ * std::cos(_yaw)) * speed;

				if (Input::IsKeyPressed(IMGN_KEY_SPACE))
				{
					if (Input::IsKeyPressed(IMGN_KEY_LEFT_SHIFT)) camPos[1] -= speed;
					else camPos[1] += speed;
				}

				// local movement -> world movement
				camPos[0] += (moveX * std::cos(_yaw) + moveZ * std::sin(_yaw)) * speed;
				camPos[2] += (-moveX * std::sin(_yaw) + moveZ * std::cos(_yaw)) * speed;

				transform->position = camPos;

				//rotation
				//auto [deltaX, deltaY] = Input::GetMouseDelta();
				float pitch = 45.f * deltaY / ImgnApp::Get().GetWindow().GetHeight();
				float yaw = 45.f * ImgnApp::Get().GetWindow().GetAspectRatio() * deltaX / ImgnApp::Get().GetWindow().GetWidth();

				_yaw += Math::Radians(yaw);

				transform->rotation[0] += pitch;
				transform->rotation[1] += yaw;// (pitch, yaw);
			}

		public:
			void Sleep()
			{
				IMGN_TRACE("sleep");
				cam = GetComponent<CameraComponent>();
				transform = GetComponent<TransformComponent>();
			}

			void WakeUp()
			{

			}

			void Dream(Time pTime)
			{
				UpdateCamera(pTime);
			}
		};

		_mainCamera->AddComponent<ScriptComponent>()->Bind<TestScript>();
		//_camera.SetFarPlane(10000.f);
		//_camera.SetViewportSize(_window->GetWidth(), _window->GetHeight());

		GBufferUBO gBufferUBO
		{
			.viewProj = Math::Inverse(cameraTransform->GetTransform()) * camera->camera.GetProjection()
		};

		gBufferUBOHandle = _renderer->CreateUniformBuffer(&gBufferUBO, sizeof(GBufferUBO));

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

		ImGui::Begin("Scene Hierarchy");
		for (auto& entityPtr : _activeScene->GetEntities())
		{
			Entity* entity = entityPtr.get();

			bool selected = entity == _selectedEntity;

			if (ImGui::Selectable(entity->GetName().c_str(), selected))
			{
				_selectedEntity = entity;
			}
		}
		ImGui::End();

		ImGui::Begin("Inspector");
		{
			if (_selectedEntity)
			{
				ImGui::Text("%s", _selectedEntity->GetName().c_str());

				if (auto* transform = _selectedEntity->GetComponent<TransformComponent>())
				{
					if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::DragFloat3("Position", transform->position.data(), 0.1f);
						ImGui::DragFloat3("Rotation", transform->rotation.data(), 0.5f);
						ImGui::DragFloat3("Scale", transform->scale.data(), 0.01f);
					}
				}

				if (CameraComponent* cameraComp = _selectedEntity->GetComponent<CameraComponent>()) //means its a camera
				{
					ImGui::DragFloat("Camera Speed", &cameraComp->cameraSpeed, 5.f);

					if (cameraComp->cameraType == CameraType::Perspective)
					{
						if (ImGui::DragFloat("FOV", &cameraComp->camera.GetFOV(), 5.f) ||
							ImGui::DragFloat("Near", &cameraComp->camera.GetNearPlane(), 5.f) ||
							ImGui::DragFloat("Far", &cameraComp->camera.GetFarPlane(), 5.f) ||
							ImGui::DragFloat("Aspect", &cameraComp->camera.GetAspect(), 5.f))
						{
							cameraComp->camera.RecalculateProjection();
						}
					}

					if (cameraComp->cameraType == CameraType::Orthographic)
					{
						if (ImGui::DragFloat("Near", &cameraComp->camera.GetNearPlane(), 5.f) ||
							ImGui::DragFloat("Far", &cameraComp->camera.GetFarPlane(), 5.f) ||
							ImGui::DragFloat("Aspect", &cameraComp->camera.GetAspect(), 5.f) ||
							ImGui::DragFloat("Ortho Right", &cameraComp->camera.GetOrthoRight(), 5.f) ||
							ImGui::DragFloat("Ortho Left", &cameraComp->camera.GetOrthoLeft(), 5.f) ||
							ImGui::DragFloat("Ortho Top", &cameraComp->camera.GetOrthoTop(), 5.f) ||
							ImGui::DragFloat("Ortho Bottom", &cameraComp->camera.GetOrthoBottom(), 5.f))
						{
							cameraComp->camera.RecalculateProjection();
						}
					}
				}

				if (auto* mesh = _selectedEntity->GetComponent<MeshComponent>())
				{
					if (ImGui::CollapsingHeader("Mesh"))
					{
						ImGui::Text("Mesh: %u", mesh->mesh);

						/*ImGui::Checkbox(
							"Visible",
							&mesh->visible
						);*/
					}
				}
			}
		}
		ImGui::End();

		if (!_sceneWindow)
		{
			_sceneWindow = static_cast<vk::DescriptorSet>(ImGui_ImplVulkan_AddTexture(*_renderer->GetTextureSampler(), **_renderer->GetRenderGraphImage("LitScene").image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
		}

		ImTextureID textureID = static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(static_cast<VkDescriptorSet>(_sceneWindow)));

		ImGui::Begin("SceneView");
		ImVec2 sceneViewSize = ImGui::GetContentRegionAvail();
		_sceneWidth = static_cast<uint32_t>(sceneViewSize.x); _sceneHeight = static_cast<uint32_t>(sceneViewSize.y);
		ImGui::Image(ImTextureRef(textureID), ImVec2(_window->GetWidth(), _window->GetHeight()));
		ImGui::End();
	}

	void EditorLayer::Dream(Time pTime)
	{
		//UpdateCamera(pTime);

		GBufferUBO gBufferUBO
		{
			.viewProj = Math::Inverse(_mainCamera->GetComponent<TransformComponent>()->GetTransform()) * _mainCamera->GetComponent<CameraComponent>()->camera.GetProjection()
		};
		_renderer->MapBufferData(gBufferUBOHandle, &gBufferUBO, sizeof(GBufferUBO));
		//IMGN_INFO("DeltaTime {}s : {}ms", pTime.Seconds(), pTime.MiliSeconds());

		_activeScene->Dream(pTime);

		_renderer->ExecuteGraph();
		_renderer->BlitToSwapchain("LitScene");
	}

	void EditorLayer::OnEvent(Event& pEvent)
	{
	}
}