#include "EditorLayer.hpp"
#include "EditorCamera.h"
#include "Utils/EditorUtils.h"

#include <Imgn/SceneSerializer.h>

#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_vulkan.h"

namespace Imgn
{
	namespace
	{
		// Radical-inverse Halton sequence (Elo / common TAA practice).
		float Halton(uint32_t index, uint32_t base)
		{
			float f = 1.f;
			float r = 0.f;
			while (index > 0)
			{
				f /= static_cast<float>(base);
				r += f * static_cast<float>(index % base);
				index /= base;
			}
			return r;
		}
	}

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
		constexpr uint32_t sampleCount = 8;
		// 1-based index into an 8-sample Halton(2,3) cycle
		const uint32_t index = (_jitterFrameIndex++ % sampleCount) + 1;
		// Map [0,1] -> pixel-center offset [-0.5, 0.5]
		return { Halton(index, 2) - 0.5f, Halton(index, 3) - 0.5f };
	}
	vec2 EditorLayer::GetProjectionJitter(uint32_t pWidth, uint32_t pHeight)
	{
		vec2 pixelJitter = GetJitterSample();
		// Elo: valid projection jitter is +/-1/(2w), +/-1/(2h) == +/-0.5/w, +/-0.5/h
		return { pixelJitter[0] / static_cast<float>(pWidth), pixelJitter[1] / static_cast<float>(pHeight) };
	}
	void EditorLayer::Sleep()
	{
		GLTFLoader& loader = GLTFLoader::Get();
		ImgnModel sponza = loader.LoadModel("../../Models/Sponza/glTF/Sponza.gltf", *_renderer);
		ImgnModel testGlb = loader.LoadModel("../../Models/Vroid/Test.gltf", *_renderer);

		RenderPass gBuffer
		{
			.name = "G-BufferPass",
			.bindPoint = vk::PipelineBindPoint::eGraphics,
			.imageOUT =
			{
				_renderer->CreateRGImageDesc("G-BufferAlbedo", _window->GetWidth(), _window->GetHeight(), vk::Format::eR8G8B8A8Srgb),
				_renderer->CreateRGImageDesc("G-BufferNormal", _window->GetWidth(), _window->GetHeight(), vk::Format::eR8G8B8A8Unorm),
				_renderer->CreateRGImageDesc("G-BufferMaterial", _window->GetWidth(), _window->GetHeight(), vk::Format::eR8G8B8A8Unorm),
				_renderer->CreateRGImageDesc("G-BufferEmissive", _window->GetWidth(), _window->GetHeight(), vk::Format::eR8G8B8A8Srgb),
				_renderer->CreateRGImageDesc("G-BufferVelocity", _window->GetWidth(), _window->GetHeight(), vk::Format::eR16G16Sfloat),
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
					ctx.CreateRenderingAttachmentInfo("G-BufferVelocity"),
				};

				vk::RenderingAttachmentInfo depthAttachment = ctx.CreateRenderingAttachmentInfo("Depth");

				ctx.BeginRendering(_window->GetWidth(), _window->GetHeight(), colorAttachments, &depthAttachment);
				ctx.BindPipeline(vk::PipelineBindPoint::eGraphics, *_renderer->GetPipelines().gBufferPipeline);
				ctx.BindDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), 1, *_renderer->GetTextureDescriptorSet());
				ctx.SetViewport(_window->GetWidth(), _window->GetHeight());
				ctx.SetScissor(_window->GetWidth(), _window->GetHeight());

				vk::DescriptorBufferInfo uboInfo = ctx.CreateDescriptorBufferInfo(gBufferUBOHandle, sizeof(GBufferUBO));

				std::vector writes
				{
					ctx.CreateWriteDescriptorSet(0, vk::DescriptorType::eUniformBuffer, uboInfo),
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
				_renderer->CreateRGImageDesc("LitScene", _window->GetWidth(), _window->GetHeight(), vk::Format::eR16G16B16A16Sfloat)
			},
			.Execute = [&](Imgn::RenderContext& ctx)
			{
				ctx.BindPipeline(vk::PipelineBindPoint::eCompute, *_renderer->GetPipelines().lightingPipeline);
				ctx.BindDescriptorSet(vk::PipelineBindPoint::eCompute, _renderer->GetPipelineLayout(), 1, *_renderer->GetTextureDescriptorSet());

				LightingPC pc
				{
					.invViewProj = Math::Inverse(gBufferUBO.jitteredViewProj),
					.camPos = _sceneCamera->GetComponent<TransformComponent>()->position,
					.width = _window->GetWidth(),
					.height = _window->GetHeight()
				};

				ctx.PushConstants<LightingPC>(vk::ShaderStageFlagBits::eCompute, pc);

				{
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

				ctx.Dispatch((_window->GetWidth() + 7) / 8, (_window->GetHeight() + 7) / 8, 1);
			}
		};

		_renderer->CreateRGImageDesc("TAAHistory", _window->GetWidth(), _window->GetHeight(), vk::Format::eR16G16B16A16Sfloat);
		// Previous-frame velocity for TAA velocity rejection (same import+copy pattern as TAAHistory).
		_renderer->CreateRGImageDesc("VelocityHistory", _window->GetWidth(), _window->GetHeight(), vk::Format::eR16G16Sfloat);

		RenderPass TAA
		{
			.name = "TemporalAntiAliasing",
			.bindPoint = vk::PipelineBindPoint::eCompute,
			.imageIN =
			{
				"LitScene",
				"TAAHistory",
				"G-BufferVelocity",
				"VelocityHistory",
			},
			.imageOUT =
			{
				_renderer->CreateRGImageDesc("TAAResolved", _window->GetWidth(), _window->GetHeight(), vk::Format::eR16G16B16A16Sfloat)
			},
			.Execute = [&](Imgn::RenderContext& ctx)
			{
				ctx.BindPipeline(vk::PipelineBindPoint::eCompute, *_renderer->GetPipelines().taaPipeline);
				ctx.BindDescriptorSet(vk::PipelineBindPoint::eCompute, _renderer->GetPipelineLayout(), 1, *_renderer->GetTextureDescriptorSet());

				TAAPC pc
				{
					.historyValid = _taaHistoryValid
				};

				ctx.PushConstants<TAAPC>(vk::ShaderStageFlagBits::eCompute, pc);
				{
					std::array images =
					{
						ctx.CreateDescriptorImageInfo("LitScene"),
						ctx.CreateDescriptorImageInfo("TAAHistory"),
						ctx.CreateDescriptorImageInfo("G-BufferVelocity"),
						ctx.CreateDescriptorImageInfo("VelocityHistory"),
					};

					vk::DescriptorImageInfo litImage = ctx.CreateDescriptorImageInfo("TAAResolved", nullptr, vk::ImageLayout::eGeneral);

					vk::DescriptorImageInfo sampler = ctx.CreateSamplerInfo(_renderer->GetTAASampler());

					std::array writes
					{
						ctx.CreateWriteDescriptorSet(2, vk::DescriptorType::eSampledImage, images),
						ctx.CreateWriteDescriptorSet(3, vk::DescriptorType::eStorageImage, litImage),
						ctx.CreateWriteDescriptorSet(4, vk::DescriptorType::eSampler, sampler)
					};

					ctx.PushDescriptorSet(vk::PipelineBindPoint::eCompute, _renderer->GetPipelineLayout(), writes);
				}

				ctx.Dispatch((_window->GetWidth() + 7) / 8, (_window->GetHeight() + 7) / 8, 1);
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
			_renderer->GetMesh(meshHandle).materialBuffer = testGlb.materialBuffer;
			_renderer->GetMesh(meshHandle).materialBufferSize = testGlb.materialBufferSize;
		}

		_sceneCamera = _activeScene->CreateEntity("SceneCamera");
		CameraComponent* camera = _sceneCamera->AddComponent<CameraComponent>();
		TransformComponent* cameraTransform = _sceneCamera->GetComponent<TransformComponent>();
		camera->camera.SetViewportSize(_window->GetWidth(), _window->GetHeight());

		_sceneCamera->AddComponent<ScriptComponent>()->Bind<EditorCamera>();
		_sceneHierarchy.SetSceneContext(_activeScene);

		gBufferUBO.viewProj = GetCamView(cameraTransform) * camera->camera.GetProjection();
		gBufferUBO.prevViewProj = gBufferUBO.viewProj;

		gBufferUBOHandle = _renderer->CreateUniformBuffer(nullptr, sizeof(GBufferUBO));
	}

	void EditorLayer::WakeUp()
	{
	}

	void EditorLayer::OnImGuiRender()
	{
		ImGui::DockSpaceOverViewport();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"))
				{
					_activeScene = Shared<Scene>();
					_activeScene->OnViewportResize(_sceneWidth, _sceneHeight);
					_sceneHierarchy.SetSceneContext(_activeScene);
				}
				if (ImGui::MenuItem("Open...", "Ctrl+O"))
				{
					std::string filePath = FileDialogs::OpenFile("Imgn File (*.imgn)\0*.imgn\0");
					if (!filePath.empty())
					{
						_activeScene = Shared<Scene>();
						_activeScene->OnViewportResize(_sceneWidth, _sceneHeight);
						_sceneHierarchy.SetSceneContext(_activeScene);

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

		if (!_sceneWindow)
		{
			_sceneWindow = static_cast<vk::DescriptorSet>(ImGui_ImplVulkan_AddTexture(*_renderer->GetTextureSampler(), **_renderer->GetRenderGraphImage("TAAResolved").image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
		}

		ImTextureID textureID = static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(static_cast<VkDescriptorSet>(_sceneWindow)));

		ImGui::Begin("SceneView");
		ImVec2 sceneViewSize = ImGui::GetContentRegionAvail();
		_sceneWidth = static_cast<uint32_t>(sceneViewSize.x); _sceneHeight = static_cast<uint32_t>(sceneViewSize.y);
		_sceneCamera->GetComponent<CameraComponent>()->camera.SetViewportSize(_sceneWidth, _sceneHeight);
		ImGui::Image(ImTextureRef(textureID), ImVec2(_window->GetWidth(), _window->GetHeight()));
		ImGui::End();
	}

	void EditorLayer::Dream(Time pTime)
	{
		_activeScene->Dream(pTime);

		constexpr float JITTER_DEBUG_SCALE = 1.f;

		mat4 proj = _sceneCamera->GetComponent<CameraComponent>()->camera.GetProjection();
		vec2 jitter = GetProjectionJitter(_window->GetWidth(), _window->GetHeight());
		mat4 jitterMat = Math::Translate(Math::identity, { jitter[0] * JITTER_DEBUG_SCALE, jitter[1] * JITTER_DEBUG_SCALE, 0.f });
		gBufferUBO.jitteredViewProj = GetCamView(_sceneCamera->GetComponent<TransformComponent>()) * (proj * jitterMat);
		gBufferUBO.viewProj = GetCamView(_sceneCamera->GetComponent<TransformComponent>()) * proj;

		_renderer->MapBufferData(gBufferUBOHandle, &gBufferUBO, sizeof(GBufferUBO));

		gBufferUBO.prevViewProj = gBufferUBO.viewProj;

		_renderer->ExecuteGraph();
		_renderer->CopyRenderImage("TAAResolved", "TAAHistory");
		// After resolve: current velocity becomes previous-frame history next frame.
		_renderer->CopyRenderImage("G-BufferVelocity", "VelocityHistory");

		_taaHistoryValid = true;
		_renderer->BlitToSwapchain("TAAResolved");
	}

	void EditorLayer::OnEvent(Event& pEvent)
	{
	}
}
