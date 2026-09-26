#include "EditorLayer.hpp"
#include "EditorCamera.h"
#include "Utils/EditorUtils.h"

#include <Imgn/SceneSerializer.h>

#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_vulkan.h"

#include "ImGuizmo/ImGuizmo.h"

namespace Imgn
{
	mat4 EditorLayer::GetCamView(TransformComponent* pTransform)
	{
		const float pitch = Math::Radians(pTransform->rotation[0]);

		const float yaw = Math::Radians(pTransform->rotation[1]);

		vec3 forward =
		{
			std::sin(yaw) * std::cos(pitch),
			-std::sin(pitch),
			std::cos(yaw) * std::cos(pitch)
		};

		vec3 target =
		{
			pTransform->position[0] + forward[0],
			pTransform->position[1] + forward[1],
			pTransform->position[2] + forward[2]
		};

		return Math::LookAtLH(pTransform->position, target, { 0.f, 1.f, 0.f });
	}

	vec2 EditorLayer::GetJitterSample()
	{
		auto Halton = [](uint32_t pIndex, uint32_t pBase)
			{
				float result = 0.f;
				float fraction = 1.f;

				while (pIndex > 0)
				{
					fraction /= static_cast<float>(pBase);
					result += fraction * static_cast<float>(pIndex % pBase);
					pIndex /= pBase;
				}

				return result;
			};

		const uint32_t sample = (_jitterFrameIndex++ % 8) + 1;
		return { Halton(sample, 2) - 0.5f, Halton(sample, 3) - 0.5f };
	}
	vec2 EditorLayer::GetProjectionJitter(uint32_t pWidth, uint32_t pHeight)
	{
		const vec2 sample = GetJitterSample();
		return { 2.f * sample[0] / static_cast<float>(pWidth), 2.f * sample[1] / static_cast<float>(pHeight) };
	}
	void EditorLayer::Sleep()
	{
		const vk::Extent2D extent = _renderer->GetSwapchainExtent();

		_renderWidth = _sceneWidth = std::max(1u, extent.width);
		_renderHeight = _sceneHeight = std::max(1u, extent.height);

		GLTFLoader& loader = GLTFLoader::Get();
		ImgnModel sponza = loader.LoadModel("../../../../Models/Sponza/glTF/Sponza.gltf", *_renderer);
		ImgnModel testGlb = loader.LoadModel("../../../../Models/Vroid/Test.gltf", *_renderer);

		RenderPass gBuffer
		{
			.name = "G-BufferPass",
			.imageOUT =
			{
				_renderer->CreateRGImageDesc("G-BufferAlbedo", _renderWidth, _renderHeight, vk::Format::eR8G8B8A8Srgb),
				_renderer->CreateRGImageDesc("G-BufferNormal", _renderWidth, _renderHeight, vk::Format::eR8G8B8A8Unorm),
				_renderer->CreateRGImageDesc("G-BufferMaterial", _renderWidth, _renderHeight, vk::Format::eR8G8B8A8Unorm),
				_renderer->CreateRGImageDesc("G-BufferEmissive", _renderWidth, _renderHeight, vk::Format::eR8G8B8A8Srgb),
				_renderer->CreateRGImageDesc("G-BufferVelocity", _renderWidth, _renderHeight, vk::Format::eR16G16Sfloat),
				_renderer->CreateRGImageDesc("Depth", _renderWidth, _renderHeight, vk::Format::eD32Sfloat)
			},
			.Execute = [&, sponza](Imgn::RenderContext& ctx)
			{
				std::vector colorAttachments =
				{
					ctx.CreateRenderingAttachmentInfo("G-BufferAlbedo"),
					ctx.CreateRenderingAttachmentInfo("G-BufferNormal"),
					ctx.CreateRenderingAttachmentInfo("G-BufferMaterial"),
					ctx.CreateRenderingAttachmentInfo("G-BufferEmissive"),
					ctx.CreateRenderingAttachmentInfo("G-BufferVelocity"),
				};

				vk::RenderingAttachmentInfo depthAttachment = ctx.CreateRenderingAttachmentInfo("Depth");

				ctx.BeginRendering(_renderWidth, _renderHeight, colorAttachments, &depthAttachment);
				ctx.BindPipeline(vk::PipelineBindPoint::eGraphics, *_renderer->GetPipelines().gBufferPipeline);
				ctx.BindDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), 1, *_renderer->GetTextureDescriptorSet());
				ctx.SetViewport(_renderWidth, _renderHeight);
				ctx.SetScissor(_renderWidth, _renderHeight);

				vk::DescriptorBufferInfo uboInfo = ctx.CreateDescriptorBufferInfo(gBufferUBOHandles[_renderer->GetFrameInFlightIndex()], sizeof(GBufferUBO));
				//vk::DescriptorBufferInfo materialSBInfo = ctx.CreateDescriptorBufferInfo(sponza.materialBuffer, sponza.materialBufferSize);

				std::vector writes
				{
					ctx.CreateWriteDescriptorSet(0, vk::DescriptorType::eUniformBuffer, uboInfo),
					//ctx.CreateWriteDescriptorSet(1, vk::DescriptorType::eStorageBuffer, materialSBInfo),
				};

				ctx.PushDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), writes);

				for (auto& entity : _activeScene->GetEntities())
				{
					if (!entity->IsActive()) continue;

					if (TransformComponent* transform = entity->GetComponent<TransformComponent>())
					{
						if (MeshComponent* meshComp = entity->GetComponent<MeshComponent>())
						{
							auto& mesh = _renderer->GetMesh(meshComp->mesh);
							if (!meshComp->visible) continue;

							ctx.BindMesh(meshComp->mesh);
							vk::DescriptorBufferInfo materialSBInfo = ctx.CreateDescriptorBufferInfo(mesh.materialBuffer, mesh.materialBufferSize);

							std::vector writes
							{
								ctx.CreateWriteDescriptorSet(1, vk::DescriptorType::eStorageBuffer, materialSBInfo),
							};

							ctx.PushDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), writes);

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

					//for (auto& children : entity->GetChildren())
					//{
					//	if (!children->IsActive()) continue;

					//	if (TransformComponent* transform = children->GetComponent<TransformComponent>())
					//	{
					//		if (MeshComponent* meshComp = children->GetComponent<MeshComponent>())
					//		{
					//			auto& mesh = _renderer->GetMesh(meshComp->mesh);
					//			if (!meshComp->visible) continue;

					//			ctx.BindMesh(meshComp->mesh);
					//			vk::DescriptorBufferInfo materialSBInfo = ctx.CreateDescriptorBufferInfo(mesh.materialBuffer, mesh.materialBufferSize);

					//			std::vector writes
					//			{
					//				ctx.CreateWriteDescriptorSet(1, vk::DescriptorType::eStorageBuffer, materialSBInfo),
					//			};

					//			ctx.PushDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), writes);

					//			for (ImgnPrimitive& prim : _renderer->GetMesh(meshComp->mesh).primitives)
					//			{
					//				GBufferPC pc
					//				{
					//					.model = transform->GetTransform(),
					//					.materialIndex = prim.material
					//				};

					//				ctx.PushConstants<GBufferPC>(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, pc);
					//				ctx.DrawPrimitive(prim);
					//			}
					//		}
					//	}
					//}
				}

				ctx.EndRendering();
			}
		};

		RenderPass lighting
		{
			.name = "LightingPass",
			.bindPoint = vk::PipelineBindPoint::eCompute,
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
				_renderer->CreateRGImageDesc("LitScene", _renderWidth, _renderHeight, vk::Format::eR16G16B16A16Sfloat)
			},
			.Execute = [&](Imgn::RenderContext& ctx)
			{
				ctx.BindPipeline(vk::PipelineBindPoint::eCompute, *_renderer->GetPipelines().lightingPipeline);
				ctx.BindDescriptorSet(vk::PipelineBindPoint::eCompute, _renderer->GetPipelineLayout(), 1, *_renderer->GetTextureDescriptorSet());

				LightingPC pc
				{
					.invViewProj = Math::Inverse(gBufferUBO.jitteredViewProj),
					.camPos = _sceneCamera->GetComponent<TransformComponent>()->position,
					.width = _renderWidth,
					.height = _renderHeight
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

					vk::DescriptorImageInfo litImage = ctx.CreateDescriptorImageInfo("LitScene", nullptr, vk::ImageLayout::eGeneral);


					std::array writes
					{
						ctx.CreateWriteDescriptorSet(2, vk::DescriptorType::eSampledImage, images),
						ctx.CreateWriteDescriptorSet(3, vk::DescriptorType::eStorageImage, litImage)
					};

					ctx.PushDescriptorSet(vk::PipelineBindPoint::eCompute, _renderer->GetPipelineLayout(), writes);
				}

				ctx.Dispatch((_renderWidth + 7) / 8, (_renderHeight + 7) / 8, 1);
			}
		};

		_renderer->CreateRGImageDesc("TAAHistory", _renderWidth, _renderHeight, vk::Format::eR16G16B16A16Sfloat);
		_renderer->CreateRGImageDesc("G-BufferVelocityHistory", _renderWidth, _renderHeight, vk::Format::eR16G16Sfloat);

		RenderPass TAA
		{
			.name = "TemporalAntiAliasing",
			.bindPoint = vk::PipelineBindPoint::eCompute,
			.imageIN =
			{
				"LitScene",
				"TAAHistory",
				"G-BufferVelocity",
				"G-BufferVelocityHistory",
			},
			.imageOUT =
			{
				_renderer->CreateRGImageDesc("TAAResolved", _renderWidth, _renderHeight, vk::Format::eR16G16B16A16Sfloat)
			},
			.Execute = [&](Imgn::RenderContext& ctx)
			{
				ctx.BindPipeline(vk::PipelineBindPoint::eCompute, *_renderer->GetPipelines().taaPipeline);
				ctx.BindDescriptorSet(vk::PipelineBindPoint::eCompute, _renderer->GetPipelineLayout(), 1, *_renderer->GetTextureDescriptorSet());

				TAAPC pc
				{
					.historyValid = static_cast<uint32_t>(_taaHistoryValid)
				};

				ctx.PushConstants<TAAPC>(vk::ShaderStageFlagBits::eCompute, pc);
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
						ctx.CreateDescriptorImageInfo("LitScene"),
						ctx.CreateDescriptorImageInfo("TAAHistory"),
						ctx.CreateDescriptorImageInfo("G-BufferVelocity"),
						ctx.CreateDescriptorImageInfo("G-BufferVelocityHistory"),
					};

					vk::DescriptorImageInfo litImage = ctx.CreateDescriptorImageInfo("TAAResolved", nullptr, vk::ImageLayout::eGeneral);

					//sampler

					vk::DescriptorImageInfo sampler = ctx.CreateSamplerInfo(_renderer->GetTAASampler());

					std::array writes
					{
						ctx.CreateWriteDescriptorSet(2, vk::DescriptorType::eSampledImage, images),
						ctx.CreateWriteDescriptorSet(3, vk::DescriptorType::eStorageImage, litImage),
						ctx.CreateWriteDescriptorSet(4, vk::DescriptorType::eSampler, sampler)
					};

					ctx.PushDescriptorSet(vk::PipelineBindPoint::eCompute, _renderer->GetPipelineLayout(), writes);
				}

				ctx.Dispatch((_renderWidth + 7) / 8, (_renderHeight + 7) / 8, 1);
			}
		};

		_renderer->AddPass(gBuffer);
		_renderer->AddPass(lighting);
		_renderer->AddPass(TAA);
		_renderer->CompileGraph();

		_activeScene = Shared<Scene>();

		for (auto& meshHandle : sponza.meshes)
		{
			Entity* entity = _activeScene->CreateEntity("Sponza");
			MeshComponent* mesh = entity->AddComponent<Imgn::MeshComponent>(meshHandle);
			mesh->materials = sponza.materials;
			//todo remove
			_renderer->GetMesh(meshHandle).materialBuffer = sponza.materialBuffer;
			_renderer->GetMesh(meshHandle).materialBufferSize = sponza.materialBufferSize;

		}

		Entity* vroid = _activeScene->CreateEntity("Vroid");
		for (int i = 0; auto& meshHandle : testGlb.meshes)
		{

			Entity* child = vroid->AddChild(_activeScene->CreateEntity((_renderer->GetMesh(meshHandle).name)));
			MeshComponent* mesh = child->AddComponent<Imgn::MeshComponent>();
			mesh->mesh = meshHandle;
			mesh->materials = testGlb.materials;
			//todo remove
			_renderer->GetMesh(meshHandle).materialBuffer = testGlb.materialBuffer;
			_renderer->GetMesh(meshHandle).materialBufferSize = testGlb.materialBufferSize;
		}

		_editorScene = Shared<Scene>();
		_sceneCamera = _editorScene->CreateEntity("SceneCamera");
		CameraComponent* camera = _sceneCamera->AddComponent<CameraComponent>();
		TransformComponent* cameraTransform = _sceneCamera->GetComponent<TransformComponent>();
		camera->camera.SetViewportSize(_renderWidth, _renderHeight);

		_sceneCamera->AddComponent<ScriptComponent>()->Bind<EditorCamera>();
		_sceneHierarchy.SetSceneContext(_activeScene);

		//GBufferUBO gBufferUBO
		//{
		//	.viewProj = Math::Inverse(cameraTransform->GetTransform()) * camera->camera.GetProjection()
		//};

		gBufferUBO.viewProj = GetCamView(cameraTransform) * camera->camera.GetProjection();
		gBufferUBO.prevViewProj = gBufferUBO.viewProj;

		for (auto& handle : gBufferUBOHandles)
		{
			handle = _renderer->CreateUniformBuffer(nullptr, sizeof(GBufferUBO));
		}

		blenderPanel.Initialize(static_cast<HWND>(_window->GetWindowHandle()), "../../../../ExternalApps/Blender 4.4/blender.exe");
	}

	void EditorLayer::WakeUp()
	{
		blenderPanel.Shutdown();
	}

	void EditorLayer::OnImGuiRender()
	{
		EditorCamera::SetInputEnabled(false);
		ImGui::DockSpaceOverViewport();

		// Show demo options and help
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"))
				{
					_activeScene = Shared<Scene>();
					_activeScene->OnViewportResize(_sceneWidth, _sceneHeight);
					_sceneHierarchy.SetSceneContext(_activeScene);
					_taaHistoryValid = false;
				}
				if (ImGui::MenuItem("Open...", "Ctrl+O"))
				{
					std::string filePath = FileDialogs::OpenFile("Imgn File (*.imgn)\0*.imgn\0");
					if (!filePath.empty())
					{
						_activeScene = Shared<Scene>();
						_activeScene->OnViewportResize(_sceneWidth, _sceneHeight);
						_sceneHierarchy.SetSceneContext(_activeScene);
						_taaHistoryValid = false;

						SceneSerializer serializer(_activeScene);
						serializer.Deserialize(filePath);
					}
				}
				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
				{
					std::string filePath = FileDialogs::SaveFile("Imgn File (*.imgn)\0*.imgn\0");
					if (!filePath.empty())
					{
						SceneSerializer serializer(_activeScene);
						serializer.Serialize(filePath + ".imgn");
					}
				}
				if (ImGui::MenuItem("Exit")) ImgnApp::Get().Close();
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		ImGui::ShowDemoWindow();
		ImGui::Begin("Style");
		ImGui::ShowStyleEditor();
		ImGui::End();

		_sceneHierarchy.OnImGuiRender();

		DrawSceneView();

		blenderPanel.Render();
	}

	void EditorLayer::Dream(Time pTime)
	{
		ResizeSceneTargets();

		_editorScene->Dream(pTime);
		_activeScene->Dream(pTime);

		const mat4 view = GetCamView(_sceneCamera->GetComponent<TransformComponent>());
		const mat4 projection = _sceneCamera->GetComponent<CameraComponent>()->camera.GetProjection();
		const vec2 jitter = GetProjectionJitter(_renderWidth, _renderHeight);

		mat4 jitteredProjection = projection;
		jitteredProjection[8] += jitter[0];
		jitteredProjection[9] += jitter[1];

		gBufferUBO.viewProj = view * projection;
		gBufferUBO.jitteredViewProj = view * jitteredProjection;

		if (!_taaHistoryValid)
			gBufferUBO.prevViewProj = gBufferUBO.viewProj;

		_renderer->MapBufferData(gBufferUBOHandles[_renderer->GetFrameInFlightIndex()], &gBufferUBO, sizeof(GBufferUBO));

		_renderer->ExecuteGraph();
		_renderer->CopyRenderImage("TAAResolved", "TAAHistory");
		_renderer->CopyRenderImage("G-BufferVelocity", "G-BufferVelocityHistory");

		gBufferUBO.prevViewProj = gBufferUBO.viewProj;
		_taaHistoryValid = true;

		_renderer->ClearSwapchain();

		//_activeScene->Dream(pTime);
		////UpdateCamera(pTime);

		//constexpr float JITTER_DEBUG_SCALE = 1.f;

		//mat4 proj = _sceneCamera->GetComponent<CameraComponent>()->camera.GetProjection();
		//vec2 jitter = GetProjectionJitter(_renderWidth, _renderHeight);
		//mat4 jitterMat = Math::Translate(Math::identity, { jitter[0] * JITTER_DEBUG_SCALE, jitter[1] * JITTER_DEBUG_SCALE, 0.f });
		////proj[8] += jitter[0] * JITTER_DEBUG_SCALE;
		////proj[9] += jitter[1] * JITTER_DEBUG_SCALE;
		//gBufferUBO.jitteredViewProj = GetCamView(_sceneCamera->GetComponent<TransformComponent>()) * (proj * jitterMat);
		//gBufferUBO.viewProj = GetCamView(_sceneCamera->GetComponent<TransformComponent>()) * proj;

		//_renderer->MapBufferData(gBufferUBOHandles[_renderer->GetFrameInFlightIndex()], &gBufferUBO, sizeof(GBufferUBO));

		//gBufferUBO.prevViewProj = gBufferUBO.viewProj;
		////IMGN_INFO("DeltaTime {}s : {}ms", pTime.Seconds(), pTime.MiliSeconds());


		//_renderer->ExecuteGraph();
		//_renderer->CopyRenderImage("TAAResolved", "TAAHistory");
		//_renderer->CopyRenderImage("G-BufferVelocity", "G-BufferVelocityHistory");

		//_taaHistoryValid = true;
		//_renderer->BlitToSwapchain("TAAResolved");
	}

	void EditorLayer::OnEvent(Event& pEvent)
	{
	}

	void EditorLayer::ResizeSceneTargets()
	{
		if (_sceneWidth == 0 || _sceneHeight == 0) return;
		if (_sceneWidth == _renderWidth && _sceneHeight == _renderHeight) return;

		// Runs before this frame records commands using the scene images.
		_renderer->WaitIdle();

		if (_sceneWindow)
		{
			ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(_sceneWindow));
			_sceneWindow = nullptr;
		}

		_renderer->ResizeViewport(_sceneWidth, _sceneHeight);

		_renderWidth = _sceneWidth;
		_renderHeight = _sceneHeight;

		_sceneCamera->GetComponent<CameraComponent>()->camera.SetViewportSize(_renderWidth, _renderHeight);

		_taaHistoryValid = false;
		_jitterFrameIndex = 0;
	}

	void EditorLayer::DrawSceneView()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
		const bool visible = ImGui::Begin("SceneView", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::PopStyleVar();

		const ImVec2 size = ImGui::GetContentRegionAvail();

		if (visible && size.x >= 1.f && size.y >= 1.f)
		{
			const ImVec2 scale = ImGui::GetIO().DisplayFramebufferScale;

			_sceneWidth = std::max(1u, static_cast<uint32_t>(std::round(size.x * scale.x)));
			_sceneHeight = std::max(1u, static_cast<uint32_t>(std::round(size.y * scale.y)));

			if (!_sceneWindow)
			{
				auto& image = _renderer->GetRenderGraphImage("TAAResolved");
				_sceneWindow = static_cast<vk::DescriptorSet>(ImGui_ImplVulkan_AddTexture(*_renderer->GetTextureSampler(), **image.image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
			}

			const ImTextureID textureID = static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(static_cast<VkDescriptorSet>(_sceneWindow)));
			const ImVec2 position = ImGui::GetCursorScreenPos();

			ImGui::Image(ImTextureRef(textureID), size);

			if (!ImGui::IsMouseDown(ImGuiMouseButton_Right) || !ImGui::IsWindowFocused()) _cameraLookActive = false;

			// Looking must start with a right-click inside the scene image.
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) _cameraLookActive = true;

			EditorCamera::SetInputEnabled(_cameraLookActive);

			Entity* selected = _sceneHierarchy.GetSelectedEntity();
			TransformComponent* tc = selected ? selected->GetComponent<TransformComponent>() : nullptr;

			if (tc && !_cameraLookActive)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(position.x, position.y, size.x, size.y);

				mat4 view = GetCamView(_sceneCamera->GetComponent<TransformComponent>());
				mat4 projection = _sceneCamera->GetComponent<CameraComponent>()->camera.GetProjection();

				// ImGuizmo performs its own NDC-to-screen Y flip.
				projection[5] = -projection[5];

				mat4 transform = tc->GetTransform();

				ImGuizmo::Manipulate(view.data(), projection.data(), ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, transform.data());

				if (ImGuizmo::IsUsing())
tc->position = { transform[12], transform[13], transform[14] };
			}
		}
		else
		{
			_cameraLookActive = false;
		}

		ImGui::End();
	}
}