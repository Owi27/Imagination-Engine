#include "EditorLayer.hpp"
#include "EditorCamera.h"
#include "Utils/EditorUtils.h"
#include "EditorLayerTaa.inl"

#include <Imgn/SceneSerializer.h>

#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_vulkan.h"

namespace Imgn
{
	namespace
	{
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
		const uint32_t index = (_jitterFrameIndex++ % sampleCount) + 1;
		return { Halton(index, 2) - 0.5f, Halton(index, 3) - 0.5f };
	}
	vec2 EditorLayer::GetProjectionJitter(uint32_t pWidth, uint32_t pHeight)
	{
		vec2 pixelJitter = GetJitterSample();
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

							const mat4 model = transform->GetTransform();
							const mat4 prevModel = TaaObjectMv::ResolvePrevModel(transform, model);

							for (ImgnPrimitive& prim : _renderer->GetMesh(meshComp->mesh).primitives)
							{
								GBufferPC pc
								{
									.model = model,
									.prevModel = prevModel,
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

PLACEHOLDER_REST
