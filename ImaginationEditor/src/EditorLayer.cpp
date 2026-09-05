#include "EditorLayer.hpp"
#include "EditorCamera.h"
#include "Utils/EditorUtils.h"

#include <Imgn/SceneSerializer.h>

#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_vulkan.h"

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
		constexpr int sampleCount = 8;

		std::uniform_real_distribution<float> dist(-1.f, 1.f);

		return { dist(gen), dist(gen) };
	}
	vec2 EditorLayer::GetProjectionJitter(uint32_t pWidth, uint32_t pHeight)
	{
		vec2 pixelJitter = GetJitterSample();

		return { pixelJitter[0] / static_cast<float>(2 * pWidth), pixelJitter[1] / static_cast<float>(2 * pHeight) };
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
					.invViewProj = Math::Inverse(gBufferUBO.jitteredViewProj),
					.camPos = _sceneCamera->GetComponent<TransformComponent>()->position,
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

		RenderPass TAA
		{
			.name = "TemporalAntiAliasing",
			.imageIN =
			{
				"LitScene",
				"G-BufferVelocity",
				"TAAHistory"
			},
			.imageOUT =
			{
				//_renderer->CreateRGImageDesc("TAAResolved", _window->GetWidth(), _window->GetHeight(), vk::Format::eR16G16B16A16Sfloat)
			},
			.Execute = [&](Imgn::RenderContext& ctx)
			{
				ctx.BindPipeline(vk::PipelineBindPoint::eCompute, _renderer->GetLightingPipeline());
				ctx.BindDescriptorSet(vk::PipelineBindPoint::eCompute, _renderer->GetPipelineLayout(), 1, *_renderer->GetTextureDescriptorSet());

				LightingPC pc
				{
					.invViewProj = Math::Inverse(gBufferUBO.viewProj),
					.camPos = _sceneCamera->GetComponent<TransformComponent>()->position,
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
		//_renderer->AddPass(TAA);
		_renderer->CompileGraph();

		_activeScene = Shared<Scene>();

		for (auto& meshHandle : sponza.meshes)
		{
			Entity* entity = _activeScene->CreateEntity("Sponza");
			entity->AddComponent<Imgn::MeshComponent>(meshHandle);
		}

		_sceneCamera = _activeScene->CreateEntity("SceneCamera");
		CameraComponent* camera = _sceneCamera->AddComponent<CameraComponent>();
		TransformComponent* cameraTransform = _sceneCamera->GetComponent<TransformComponent>();
		camera->camera.SetViewportSize(_window->GetWidth(), _window->GetHeight());

		_sceneCamera->AddComponent<ScriptComponent>()->Bind<EditorCamera>();
		_sceneHierarchy.SetSceneContext(_activeScene);

		//GBufferUBO gBufferUBO
		//{
		//	.viewProj = Math::Inverse(cameraTransform->GetTransform()) * camera->camera.GetProjection()
		//};

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
			_sceneWindow = static_cast<vk::DescriptorSet>(ImGui_ImplVulkan_AddTexture(*_renderer->GetTextureSampler(), **_renderer->GetRenderGraphImage("LitScene").image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
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
		//UpdateCamera(pTime);

		constexpr float JITTER_DEBUG_SCALE = 10.f;

		mat4 proj = _sceneCamera->GetComponent<CameraComponent>()->camera.GetProjection();
		vec2 jitter = GetProjectionJitter(_window->GetWidth(), _window->GetHeight());
		mat4 jitterMat = Math::Translate(Math::identity, { jitter[0] * JITTER_DEBUG_SCALE, jitter[1] * JITTER_DEBUG_SCALE, 0.f });
		//proj[8] += jitter[0] * JITTER_DEBUG_SCALE;
		//proj[9] += jitter[1] * JITTER_DEBUG_SCALE;
		gBufferUBO.jitteredViewProj = GetCamView(_sceneCamera->GetComponent<TransformComponent>()) * (proj * jitterMat);
		gBufferUBO.viewProj = GetCamView(_sceneCamera->GetComponent<TransformComponent>()) * proj;

		_renderer->MapBufferData(gBufferUBOHandle, &gBufferUBO, sizeof(GBufferUBO));

		gBufferUBO.prevViewProj = gBufferUBO.viewProj;
		//IMGN_INFO("DeltaTime {}s : {}ms", pTime.Seconds(), pTime.MiliSeconds());


		_renderer->ExecuteGraph();
		_renderer->BlitToSwapchain("LitScene");
	}

	void EditorLayer::OnEvent(Event& pEvent)
	{
	}
}