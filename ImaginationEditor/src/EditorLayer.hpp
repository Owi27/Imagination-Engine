#include <Imgn.hpp>

namespace Imgn
{
    class EditorLayer : public Layer
    {
        ImgnWindow* _window;
        ImgnRenderer* _renderer;
        vk::DescriptorSet _sceneWindow = nullptr;

		float _cameraSpeed = 100.f, _yaw = 0.f;
		vec3 _camPos = { 0, 0, 0 };
		PerspectiveCamera _camera;

		uint32_t gBufferUBOHandle = 0;

		struct PointLight
		{
			vec3 pos, col;
			float range, intensity;
		};

		std::array<PointLight, 3> pointLights
		{
			PointLight
			{
				.pos = {0.f, 0.f, 0.f},
				.col = {1.f, 0.f, 0.f},
				.range = 1000.f,
				.intensity = 100.f
			},
			PointLight
			{
				.pos = {1000.f, 0.f, 0.f},
				.col = {0.f, 1.f, 1.f},
				.range = 1000.f,
				.intensity = 100.f
			},
			PointLight
			{
				.pos = {-1000.f, 0.f, 0.f},
				.col = {1.f, 0.f, 1.f},
				.range = 1000.f,
				.intensity = 100.f
			},
		};

		void UpdateCamera(Time pTime);

    public:
        EditorLayer() /*Constructor*/
        {
        }

        EditorLayer(ImgnWindow* pWindow, ImgnRenderer* pRenderer) /*Constructor*/
        {
            _renderer = pRenderer;
            _window = pWindow;

			_camera.SetFarPlane(10000.f);
			_camera.SetViewportSize(_window->GetWidth(), _window->GetHeight());
			ImgnGLTF gltf;
			ImgnModel sponza = gltf.LoadModel("../../Models/Sponza/glTF/Sponza.gltf", *_renderer);

			GBufferUBO gBufferUBO
			{
				.viewProj = _camera.GetViewProj()
			};

			gBufferUBOHandle = _renderer->CreateUniformBuffer(&gBufferUBO, sizeof(GBufferUBO));

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
				.Execute = [&, sponza](vk::raii::CommandBuffer& commandBuffer)
				{
					std::array colorAttachments =
					{
						vk::RenderingAttachmentInfo
						{
							.imageView = *_renderer->GetRenderGraphImage("G-BufferAlbedo").image.view,
							.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
							.loadOp = vk::AttachmentLoadOp::eClear,
							.storeOp = vk::AttachmentStoreOp::eStore,
							.clearValue = vk::ClearColorValue{ std::array<float, 4>{1.0f, 0.25f, 0.75f, 1.0f} },
						},
						vk::RenderingAttachmentInfo
						{
							.imageView = *_renderer->GetRenderGraphImage("G-BufferNormal").image.view,
							.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
							.loadOp = vk::AttachmentLoadOp::eClear,
							.storeOp = vk::AttachmentStoreOp::eStore,
							.clearValue = vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} },
						},
						vk::RenderingAttachmentInfo
						{
							.imageView = *_renderer->GetRenderGraphImage("G-BufferMaterial").image.view,
							.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
							.loadOp = vk::AttachmentLoadOp::eClear,
							.storeOp = vk::AttachmentStoreOp::eStore,
							.clearValue = vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} },
						},
						vk::RenderingAttachmentInfo
						{
							.imageView = *_renderer->GetRenderGraphImage("G-BufferEmissive").image.view,
							.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
							.loadOp = vk::AttachmentLoadOp::eClear,
							.storeOp = vk::AttachmentStoreOp::eStore,
							.clearValue = vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} },
						},
					};

					vk::RenderingAttachmentInfo depthAttachment
					{
						.imageView = *_renderer->GetRenderGraphImage("Depth").image.view,
						.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
						.loadOp = vk::AttachmentLoadOp::eClear,
						.storeOp = vk::AttachmentStoreOp::eStore,
						.clearValue = vk::ClearDepthStencilValue{ .0f, 0 },
					};

					vk::RenderingInfo renderingInfo
					{
						.renderArea = { {0, 0}, { _window->GetWidth(), _window->GetHeight() } },
						.layerCount = 1,
						.colorAttachmentCount = colorAttachments.size(),
						.pColorAttachments = colorAttachments.data(),
						.pDepthAttachment = &depthAttachment,
					};

					commandBuffer.beginRendering(renderingInfo);
					commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _renderer->GetGBufferPipeline());
					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), 1, *_renderer->GetTextureDescriptorSet(), {});
					commandBuffer.setViewport(0, vk::Viewport(0.0f, 0, static_cast<float>(_window->GetWidth()), static_cast<float>(_window->GetHeight()), 0.0f, 1.0f));
					commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), { static_cast<float>(_window->GetWidth()), static_cast<float>(_window->GetHeight()) }));
					//push descriptor set
					{
						//uniform buffer
						vk::DescriptorBufferInfo uboInfo
						{
							.buffer = *_renderer->GetBuffer(gBufferUBOHandle).buffer,
							.offset = 0,
							.range = sizeof(GBufferUBO)
						};

						vk::DescriptorBufferInfo materialSBInfo
						{
							.buffer = *_renderer->GetBuffer(sponza.materialBuffer).buffer,
							.offset = 0,
							.range = sponza.materialBufferSize
						};

						const std::array writes
						{
							vk::WriteDescriptorSet
							{
								.dstSet = nullptr,
								.dstBinding = 0,
								.dstArrayElement = 0,
								.descriptorCount = 1,
								.descriptorType = vk::DescriptorType::eUniformBuffer,
								.pBufferInfo = &uboInfo
							},
							vk::WriteDescriptorSet
							{
								.dstSet = nullptr,
								.dstBinding = 1,
								.dstArrayElement = 0,
								.descriptorCount = 1,
								.descriptorType = vk::DescriptorType::eStorageBuffer,
								.pBufferInfo = &materialSBInfo
							},
						};

						commandBuffer.pushDescriptorSet(vk::PipelineBindPoint::eGraphics, _renderer->GetPipelineLayout(), 0, writes);
					}

					for (uint32_t meshHandle : sponza.meshes)
					{
						commandBuffer.bindVertexBuffers(0, **_renderer->GetBuffer(_renderer->GetMesh(meshHandle).vertexBuffer).buffer, {0});
						commandBuffer.bindIndexBuffer(**_renderer->GetBuffer(_renderer->GetMesh(meshHandle).indexBuffer).buffer, 0, vk::IndexType::eUint32);

						for (auto& prim : _renderer->GetMesh(meshHandle).primitives)
						{
							GBufferPC pc
							{
								.model = Math::identity,
								.materialIndex = prim.material
							};

							commandBuffer.pushConstants<GBufferPC>(*_renderer->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, pc);
							commandBuffer.drawIndexed(prim.indexCount, 1, prim.firstIndex, prim.vertexOffset, 0);
						}
					}

					commandBuffer.endRendering();
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
				.bufferOUT =
				{
					//Renderer().MakeBufferKey("LightingUBO", 256), //edit size
					//Renderer().MakeBufferKey("LightingSBO", 256)
				},
				.Execute = [&](vk::raii::CommandBuffer& commandBuffer)
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

					commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, _renderer->GetLightingPipeline());
					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, _renderer->GetPipelineLayout(), 1, *_renderer->GetTextureDescriptorSet(), {});

					LightingPC pc
					{
						.invViewProj = Math::Inverse(_camera.GetViewProj()),
						.camPos = _camera.GetPosition(),
						.width = _window->GetWidth(),
						.height = _window->GetHeight()
					};

					commandBuffer.pushConstants<LightingPC>(*_renderer->GetPipelineLayout(), vk::ShaderStageFlagBits::eCompute, 0, pc);

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
							vk::DescriptorImageInfo
							{
								.sampler = nullptr,
								.imageView = *_renderer->GetRenderGraphImage("G-BufferAlbedo").image.view,
								.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
							},
							vk::DescriptorImageInfo
							{
								.sampler = nullptr,
								.imageView = *_renderer->GetRenderGraphImage("G-BufferNormal").image.view,
								.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
							},
							vk::DescriptorImageInfo
							{
								.sampler = nullptr,
								.imageView = *_renderer->GetRenderGraphImage("G-BufferMaterial").image.view,
								.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
							},
							vk::DescriptorImageInfo
							{
								.sampler = nullptr,
								.imageView = *_renderer->GetRenderGraphImage("G-BufferEmissive").image.view,
								.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
							},
							vk::DescriptorImageInfo
							{
								.sampler = nullptr,
								.imageView = *_renderer->GetRenderGraphImage("Depth").image.view,
								.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
							},
						};

						vk::DescriptorImageInfo litImage
						{
							.sampler = nullptr,
							.imageView = *_renderer->GetRenderGraphImage("LitScene").image.view,
							.imageLayout = vk::ImageLayout::eGeneral
						};

						const std::array writes
						{
							vk::WriteDescriptorSet //image in
							{
								.dstSet = nullptr,
								.dstBinding = 2,
								.dstArrayElement = 0,
								.descriptorCount = static_cast<uint32_t>(images.size()),
								.descriptorType = vk::DescriptorType::eSampledImage,
								.pImageInfo = images.data()
							},
							vk::WriteDescriptorSet //out
							{
								.dstSet = nullptr,
								.dstBinding = 3,
								.dstArrayElement = 0,
								.descriptorCount = 1,
								.descriptorType = vk::DescriptorType::eStorageImage,
								.pImageInfo = &litImage
							},
						};

						commandBuffer.pushDescriptorSet(vk::PipelineBindPoint::eCompute, _renderer->GetPipelineLayout(), 0, writes);
					}

					commandBuffer.dispatch(std::ceil(_window->GetWidth() / 8), std::ceil(_window->GetHeight() / 8), 1);
				}
			};


			_renderer->AddPass(gBuffer);
			_renderer->AddPass(lighting);
			_renderer->CompileGraph();
        }

        ~EditorLayer() /*Destructor*/
        {
        }

        /*Copy Constructor*/
        EditorLayer(const EditorLayer& pOther) = default;

        /*Copy Assignment Operator*/
        EditorLayer& operator=(const EditorLayer& pOther) = default;

        /*Move Constructor*/
        EditorLayer(EditorLayer&& pOther) noexcept = default;

        /*Move Assignment Operator*/
        EditorLayer& operator=(EditorLayer&& pOther) noexcept = default;

        /*Class Functions*/
        virtual void Sleep() override;
        virtual void WakeUp() override;
        virtual void OnImGuiRender() override;
        virtual void Dream(Time pTime) override;
        virtual void OnEvent(Event& pEvent) override;

    };
}