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
		vec3 forward = Math::Rotate(vec3{ 0.f, 0.f, 1.f }, pTransform->rotation);
		vec3 target = pTransform->position + forward;

		return Math::LookAtLH(pTransform->position, target, { 0.f, 1.f, 0.f });
	}

	vec2 EditorLayer::GetJitterSample()
	{
		auto Halton = [](uint32_t pIndex, uint32_t pBase)
			{
				float result = 0.f, fraction = 1.f;

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
		ImgnModel sponza = loader.LoadModel(FileSystem::Assets() / "Models/Sponza/glTF/Sponza.gltf", *_renderer);
		ImgnModel testGlb = loader.LoadModel(FileSystem::Assets() / "Models/Vroid/Test.gltf", *_renderer);


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
				_renderer->CreateRGImageDesc("EntityIDs", _renderWidth, _renderHeight, vk::Format::eR32G32Uint),
				_renderer->CreateRGImageDesc("Depth", _renderWidth, _renderHeight, vk::Format::eD32Sfloat)
			},
			.Execute = [&](Imgn::RenderContext& ctx)
			{
				std::vector colorAttachments =
				{
					ctx.CreateRenderingAttachmentInfo("G-BufferAlbedo"),
					ctx.CreateRenderingAttachmentInfo("G-BufferNormal"),
					ctx.CreateRenderingAttachmentInfo("G-BufferMaterial"),
					ctx.CreateRenderingAttachmentInfo("G-BufferEmissive"),
					ctx.CreateRenderingAttachmentInfo("G-BufferVelocity"),
					ctx.CreateRenderingAttachmentInfo("EntityIDs"),
				};

				vk::RenderingAttachmentInfo depthAttachment = ctx.CreateRenderingAttachmentInfo("Depth");

				ctx.BeginRendering(_renderWidth, _renderHeight, colorAttachments, &depthAttachment);
				ctx.BindPipeline(vk::PipelineBindPoint::eGraphics, *_renderer->GetPipelines().gBufferPipeline);
				ctx.BindDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), 1, *_renderer->GetTextureDescriptorSet());
				ctx.SetViewport(_renderWidth, _renderHeight);
				ctx.SetScissor(_renderWidth, _renderHeight);

				vk::DescriptorBufferInfo uboInfo = ctx.CreateDescriptorBufferInfo(gBufferUBOHandles[_renderer->GetFrameInFlightIndex()], sizeof(GBufferUBO));

				std::vector writes
				{
					ctx.CreateWriteDescriptorSet(0, vk::DescriptorType::eUniformBuffer, uboInfo),
				};

				ctx.PushDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), writes);

				for (auto& entity : _activeScene->GetEntities())
				{
					if (!entity->IsActive()) continue;

					TransformComponent* transform = entity->GetComponent<TransformComponent>();
					if (!transform) continue;

					MeshComponent* meshComp = entity->GetComponent<MeshComponent>();
					if (!meshComp || !meshComp->visible) continue;

					MaterialComponent* materialComp = entity->GetComponent<MaterialComponent>();
					if (!materialComp) continue;

					materialComp->SyncMaterialBuffer(*_renderer);

					const Buffer* materialBuffer = materialComp->GetMaterialBuffer();
					if (!materialBuffer) continue;

					ctx.BindMesh(*meshComp);

					vk::DescriptorBufferInfo materialSBInfo = ctx.CreateDescriptorBufferInfo(*materialBuffer, materialComp->GetMaterialBufferSize());

					std::vector materialWrites
					{
						ctx.CreateWriteDescriptorSet(1, vk::DescriptorType::eStorageBuffer, materialSBInfo),
					};

					ctx.PushDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), materialWrites);

					for (const Primitive& prim : meshComp->GetPrimitives())
					{
						GBufferPC pc
						{
							.model = transform->GetTransform(),
							.prevModel = transform->prevTransform,
							.materialIndex = prim.materialSlot,
							.entityIDLow = static_cast<uint32_t>(entity->GetID() & 0xFFFFFFFFull),
							.entityIDHigh = static_cast<uint32_t>(entity->GetID() >> 32)
						};

						ctx.PushConstants<GBufferPC>(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, pc);
						ctx.DrawPrimitive(prim);
					}

					entity->GetComponent<TransformComponent>()->prevTransform = entity->GetComponent<TransformComponent>()->GetTransform();
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
				"PointShadowDepthArray",
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
					//point lights


					std::array images =
					{
						ctx.CreateDescriptorImageInfo("G-BufferAlbedo"),
						ctx.CreateDescriptorImageInfo("G-BufferNormal"),
						ctx.CreateDescriptorImageInfo("G-BufferMaterial"),
						ctx.CreateDescriptorImageInfo("G-BufferEmissive"),
						ctx.CreateDescriptorImageInfo("Depth"),
					};

					vk::DescriptorBufferInfo pointLights = ctx.CreateDescriptorBufferInfo(_pointLightBuffer, _pointLights.size() * sizeof(PointLight));
					vk::DescriptorImageInfo litImage = ctx.CreateDescriptorImageInfo("LitScene", nullptr, vk::ImageLayout::eGeneral);

					vk::DescriptorImageInfo sampler = ctx.CreateSamplerInfo(_renderer->GetPointSampler());

					vk::DescriptorImageInfo pointShadowDepthArray = ctx.CreateDescriptorImageInfo("PointShadowDepthArray");

					std::array writes
					{
						ctx.CreateWriteDescriptorSet(1, vk::DescriptorType::eStorageBuffer, pointLights),
						ctx.CreateWriteDescriptorSet(2, vk::DescriptorType::eSampledImage, images),
						ctx.CreateWriteDescriptorSet(3, vk::DescriptorType::eStorageImage, litImage),
						ctx.CreateWriteDescriptorSet(4, vk::DescriptorType::eSampler, sampler),
						ctx.CreateWriteDescriptorSet(5, vk::DescriptorType::eSampledImage, pointShadowDepthArray),
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

		RenderPass shadowPass //todo: magic numbers
		{
			.name = "ShadowPass",
			.imageOUT =
			{
				_renderer->CreateRGImageDesc("PointShadowDepthArray", 1024, 1024, vk::Format::eD32Sfloat, vk::ImageViewType::eCubeArray, static_cast<uint32_t>(_pointLights.size()) * 6),
			},
			.Execute = [&](Imgn::RenderContext& ctx)
			{
				ctx.BindPipeline(vk::PipelineBindPoint::eGraphics, *_renderer->GetPipelines().shadowPipeline);
				ctx.BindDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), 1, *_renderer->GetTextureDescriptorSet());
				ctx.SetViewport(1024, 1024);
				ctx.SetScissor(1024, 1024);

				const uint32_t frameIndex = _renderer->GetFrameInFlightIndex();

				for (uint32_t lightIndex = 0; lightIndex < _pointLights.size(); lightIndex++)
				{
					const PointLight& light = _pointLights[lightIndex];

					mat4 projection = Math::PerspectiveVKLH(Math::Radians(90.f), 1.f, .1f, light.posRange[3]);

					for (uint32_t face = 0; face < 6; face++)
					{
						const uint32_t layer = lightIndex * 6 + face;

						mat4 view = Math::LookAtLH({ light.posRange[0], light.posRange[1], light.posRange[2] }, vec3{ light.posRange[0], light.posRange[1], light.posRange[2] } + PointShadowDirections[face], PointShadowUp[face]);

						ShadowUBO shadowUBO
						{
							.viewProj = view * projection,
							.lightPosition = {light.posRange[0], light.posRange[1], light.posRange[2]},
							.farPlane = light.posRange[3]
						};

						_renderer->MapBufferData(_pointShadowUBOHandles[frameIndex][layer], &shadowUBO, sizeof(ShadowUBO));

						vk::DescriptorBufferInfo shadowUBOInfo = ctx.CreateDescriptorBufferInfo(_pointShadowUBOHandles[frameIndex][layer], sizeof(ShadowUBO));

						std::array writes
						{
							ctx.CreateWriteDescriptorSet(0, vk::DescriptorType::eUniformBuffer, shadowUBOInfo)
						};

						ctx.PushDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), writes);

						vk::RenderingAttachmentInfo depthAttachment = ctx.CreateRenderingAttachmentInfo("PointShadowDepthArray", layer);
						std::span<const vk::RenderingAttachmentInfo> colorAttachments;

						ctx.BeginRendering(1024, 1024, colorAttachments, &depthAttachment);

						for (auto& entity : _activeScene->GetEntities())
						{
							if (!entity->IsActive()) continue;

							TransformComponent* transform = entity->GetComponent<TransformComponent>();
							if (!transform) continue;

							MeshComponent* mesh = entity->GetComponent<MeshComponent>();
							if (!mesh || !mesh->visible) continue;

							ctx.BindMesh(*mesh);

							ShadowPC pc
							{
								.model = transform->GetTransform()
							};

							ctx.PushConstants<ShadowPC>(vk::ShaderStageFlagBits::eVertex, pc);

							for (const Primitive& primitive : mesh->GetPrimitives()) ctx.DrawPrimitive(primitive);


						}

						ctx.EndRendering();
					}
				}
			} 
		};

		const uint32_t faceCount = static_cast<uint32_t>(_pointLights.size()) * 6;

		for (std::vector<uint32_t>& handles : _pointShadowUBOHandles)
		{
			handles.resize(faceCount);

			for (uint32_t i = 0; i < faceCount; i++) handles[i] = _renderer->CreateUniformBuffer(nullptr, sizeof(ShadowUBO));
		}

		_pointLightBuffer = _renderer->CreateStorageBuffer(_pointLights.data(), _pointLights.size() * sizeof(PointLight));

		_renderer->AddPass(gBuffer);
		_renderer->AddPass(lighting);
		_renderer->AddPass(TAA);
		_renderer->AddPass(shadowPass);
		_renderer->CompileGraph();

		_activeScene = Shared<Scene>();

		//sponza
		for (auto& meshHandle : sponza.meshes)
		{
			Entity* entity = _activeScene->CreateEntity("Sponza");
			MeshComponent* mesh = entity->AddComponent<MeshComponent>();
			mesh->SetMesh(meshHandle.name, FileSystem::Assets() / "Models/Sponza/glTF/Sponza.gltf", std::move(meshHandle.vertexBuffer), std::move(meshHandle.indexBuffer), std::move(meshHandle.primitives));
			MaterialComponent* materialComponent = entity->AddComponent<MaterialComponent>();
			materialComponent->SetMaterials(sponza.materials);
		}

		Entity* vroid = _activeScene->CreateEntity("Vroid");
		for (auto& meshHandle : testGlb.meshes)
		{
			Entity* child = vroid->AddChild(_activeScene->CreateEntity(meshHandle.name));
			MeshComponent* mesh = child->AddComponent<MeshComponent>();
			mesh->SetMesh(meshHandle.name, FileSystem::Assets() / "Models/Sponza/glTF/Sponza.gltf", std::move(meshHandle.vertexBuffer), std::move(meshHandle.indexBuffer), std::move(meshHandle.primitives));

			MaterialComponent* materialComponent = child->AddComponent<MaterialComponent>();
			materialComponent->SetMaterials(testGlb.materials);
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

		//blenderPanel.Initialize(static_cast<HWND>(_window->GetWindowHandle()), FileSystem::Assets() / "ExternalApps/Blender 4.4/blender.exe");
	}

	void EditorLayer::WakeUp()
	{
		if (_activeScene) _activeScene->Clear();
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
		_hoveredEntityID = _renderer->GetEntityIDReadback();
		_hoveredEntity = _activeScene->GetEntity(_hoveredEntityID);

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

			if (ImGui::IsItemHovered())
			{
				const ImVec2 mouse = ImGui::GetMousePos();
				const ImVec2 framebufferScale = ImGui::GetIO().DisplayFramebufferScale;

				float localX = mouse.x - position.x;
				float localY = mouse.y - position.y;

				if (localX >= 0.f && localY >= 0.f && localX < size.x && localY < size.y)
				{
					uint32_t pixelX = static_cast<uint32_t>(localX * framebufferScale.x);
					uint32_t pixelY = static_cast<uint32_t>(localY * framebufferScale.y);

					pixelX = std::min(pixelX, _renderWidth - 1);
					pixelY = std::min(pixelY, _renderHeight - 1);

					_renderer->QueueEntityIDReadback(pixelX, pixelY);
				}
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver())
			{
				_sceneHierarchy.SetSelectedEntity(_hoveredEntity);
			}

			if (!ImGui::IsMouseDown(ImGuiMouseButton_Right) || !ImGui::IsWindowFocused()) _cameraLookActive = false;

			// Looking must start with a right-click inside the scene image.
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) _cameraLookActive = true;

			EditorCamera::SetInputEnabled(_cameraLookActive);

			//gizmo
			_gizmoType = ImGuizmo::OPERATION::ROTATE; //ignore hard set
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

				ImGuizmo::Manipulate(view.data(), projection.data(), (ImGuizmo::OPERATION)_gizmoType, ImGuizmo::LOCAL, transform.data());

				if (ImGuizmo::IsUsing() && _gizmoType > -1)
				{
					vec4 translation, rotation, scale;
					Math::Decompose(transform, translation, rotation, scale);

					tc->position = { translation[0], translation[1], translation[2] };
					tc->rotation = rotation;
					tc->scale = { scale[0], scale[1], scale[2] };
				}
			}
		}
		else
		{
			_cameraLookActive = false;
		}

		ImGui::End();
	}
}