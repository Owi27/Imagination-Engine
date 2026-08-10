#include <Imgn.hpp>

struct TestLayer : public Imgn::Layer
{
	TestLayer() : Layer("Test")
	{

	}

	virtual void Dream(Imgn::Time pTime) override
	{
		//IMGN_INFO("DeltaTime {}s : {}ms", pTime.Seconds(), pTime.MiliSeconds());
	}

	/* Class Functions */
};

class Daydream : public Imgn::ImgnApp
{
public:
	Daydream()
	{
		AddLayer(Unique<TestLayer>());

		Imgn::ImgnGLTF gltf;
		Imgn::ImgnModel sponza = gltf.LoadModel("../../Models/Sponza/glTF/Sponza.gltf", Renderer());

		std::vector vertices =
		{
			Vertex
			{
				.pos = { 0.f, .5f, 0.f}
			},
			Vertex
			{
				.pos = { .5f, -.5f, 0.f}
			},
			Vertex
			{
				.pos = { -.5f, -.5f, 0.f}
			}
		};

		//uint32_t vertexBuffer = Renderer().CreateVertexBuffer(vertices);

		Imgn::RenderPass gBuffer
		{
			.name = "G-BufferPass",
			.imageOUT =
			{
				Renderer().MakeImageKey("G-BufferAlbedo", GetWindow().GetWidth(), GetWindow().GetHeight()),
				Renderer().MakeImageKey("G-BufferNormal", GetWindow().GetWidth(), GetWindow().GetHeight()),
				Renderer().MakeImageKey("G-BufferMaterial", GetWindow().GetWidth(), GetWindow().GetHeight()),
				Renderer().MakeImageKey("G-BufferEmissive", GetWindow().GetWidth(), GetWindow().GetHeight()),
				Renderer().MakeImageKey("Depth", GetWindow().GetWidth(), GetWindow().GetHeight())
			},
			.bufferOUT =
			{
				Renderer().MakeBufferKey("G-BufferUBO", sizeof(Imgn::GBufferUBO)), //edit size
			},
			.Execute = [&, sponza](vk::raii::CommandBuffer& commandBuffer)
			{
				std::array colorAttachments =
				{
					vk::RenderingAttachmentInfo
					{
						.imageView = *Renderer().GetRenderGraphImage(Renderer().MakeImageKey("G-BufferAlbedo", GetWindow().GetWidth(), GetWindow().GetHeight())).image.view,
						.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
						.loadOp = vk::AttachmentLoadOp::eClear,
						.storeOp = vk::AttachmentStoreOp::eStore,
						.clearValue = vk::ClearColorValue{ std::array<float, 4>{1.0f, 0.25f, 0.75f, 1.0f} },
					},
					vk::RenderingAttachmentInfo
					{
						.imageView = *Renderer().GetRenderGraphImage(Renderer().MakeImageKey("G-BufferNormal", GetWindow().GetWidth(), GetWindow().GetHeight())).image.view,
						.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
						.loadOp = vk::AttachmentLoadOp::eClear,
						.storeOp = vk::AttachmentStoreOp::eStore,
						.clearValue = vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} },
					},
					vk::RenderingAttachmentInfo
					{
						.imageView = *Renderer().GetRenderGraphImage(Renderer().MakeImageKey("G-BufferMaterial", GetWindow().GetWidth(), GetWindow().GetHeight())).image.view,
						.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
						.loadOp = vk::AttachmentLoadOp::eClear,
						.storeOp = vk::AttachmentStoreOp::eStore,
						.clearValue = vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} },
					},
					vk::RenderingAttachmentInfo
					{
						.imageView = *Renderer().GetRenderGraphImage(Renderer().MakeImageKey("G-BufferEmissive", GetWindow().GetWidth(), GetWindow().GetHeight())).image.view,
						.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
						.loadOp = vk::AttachmentLoadOp::eClear,
						.storeOp = vk::AttachmentStoreOp::eStore,
						.clearValue = vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} },
					},
				};

				vk::RenderingAttachmentInfo depthAttachment
				{
					.imageView = *Renderer().GetRenderGraphImage(Renderer().MakeImageKey("Depth", GetWindow().GetWidth(), GetWindow().GetHeight())).image.view,
					.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
					.loadOp = vk::AttachmentLoadOp::eClear,
					.storeOp = vk::AttachmentStoreOp::eStore,
					.clearValue = vk::ClearDepthStencilValue{ .0f, 0 },
				};

				vk::RenderingInfo renderingInfo
				{
					.renderArea = { {0, 0}, { GetWindow().GetWidth(), GetWindow().GetHeight() } },
					.layerCount = 1,
					.colorAttachmentCount = colorAttachments.size(),
					.pColorAttachments = colorAttachments.data(),
					.pDepthAttachment = &depthAttachment,
				};

				commandBuffer.beginRendering(renderingInfo);
				commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, Renderer().GetGBufferPipeline());
				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, Renderer().GetPipelineLayout(), 1, *Renderer().GetTextureDescriptorSet(), {});
				commandBuffer.setViewport(0, vk::Viewport(0.0f, 0, static_cast<float>(GetWindow().GetWidth()), static_cast<float>(GetWindow().GetHeight()), 0.0f, 1.0f));
				commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), { static_cast<float>(GetWindow().GetWidth()), static_cast<float>(GetWindow().GetHeight()) }));
				//push descriptor set
				{
					//uniform buffer
					vk::DescriptorBufferInfo uboInfo
					{
						.buffer = *Renderer().GetRenderGraphBuffer(Renderer().MakeBufferKey("G-BufferUBO", sizeof(Imgn::GBufferUBO))).buffer.buffer,
						.offset = 0,
						.range = sizeof(Imgn::GBufferUBO)
					};
					
					vk::DescriptorBufferInfo materialSBInfo
					{
						.buffer = *Renderer().GetBuffer(sponza.materialBuffer).buffer,
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

					commandBuffer.pushDescriptorSet(vk::PipelineBindPoint::eGraphics, Renderer().GetPipelineLayout(), 0, writes);
				}
				
				for (uint32_t meshHandle : sponza.meshes)
				{
					commandBuffer.bindVertexBuffers(0, **Renderer().GetBuffer(Renderer().GetMesh(meshHandle).vertexBuffer).buffer, {0});
					commandBuffer.bindIndexBuffer(**Renderer().GetBuffer(Renderer().GetMesh(meshHandle).indexBuffer).buffer, 0, vk::IndexType::eUint32);

					for (auto& prim : Renderer().GetMesh(meshHandle).primitives)
					{
						Imgn::GBufferPC pc
						{
							.model = std::bit_cast<std::array<float, 16>>(GW::MATH::GIdentityMatrixF),
							.materialIndex = prim.material
						};

						commandBuffer.pushConstants<Imgn::GBufferPC>(*Renderer().GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, pc);
						commandBuffer.drawIndexed(prim.indexCount, 1, prim.firstIndex, prim.vertexOffset, 0);
					}
				}

				commandBuffer.endRendering();
			}
		};

		Imgn::RenderPass lighting
		{
			.name = "LightingPass",
			.imageIN =
			{
				Renderer().MakeImageKey("G-BufferAlbedo", GetWindow().GetWidth(), GetWindow().GetHeight()),
				Renderer().MakeImageKey("G-BufferNormal", GetWindow().GetWidth(), GetWindow().GetHeight()),
				Renderer().MakeImageKey("G-BufferMaterial", GetWindow().GetWidth(), GetWindow().GetHeight()),
				Renderer().MakeImageKey("G-BufferEmissive", GetWindow().GetWidth(), GetWindow().GetHeight()),
				Renderer().MakeImageKey("Depth", GetWindow().GetWidth(), GetWindow().GetHeight())
			},
			.imageOUT =
			{
				Renderer().MakeImageKey("LitScene", GetWindow().GetWidth(), GetWindow().GetHeight()),
			},
			.bufferOUT =
			{
				Renderer().MakeBufferKey("LightingUBO", 256), //edit size
				Renderer().MakeBufferKey("LightingSBO", 256)
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
				};

				commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, Renderer().GetLightingPipeline());*/
				
				//push descriptor set
				{
					//uniform buffer
					/*vk::DescriptorBufferInfo uboInfo
					{
						.buffer = *Renderer().GetRenderGraphBuffer("G-BufferUBO").buffer.buffer,
						.offset = 0,
						.range = 192
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
					};

					commandBuffer.pushDescriptorSet(vk::PipelineBindPoint::eGraphics, Renderer().GetPipelineLayout(), 0, writes);*/
				}

				//commandBuffer.dispatch(std::ceil(GetWindow().GetWidth() / 8), std::ceil(GetWindow().GetHeight() / 8), 1);
			}
		};


		Renderer().AddPass(gBuffer);
		Renderer().AddPass(lighting);
		Renderer().CompileGraph();

		Imgn::PerspectiveCamera cam;
		cam.SetViewportSize(GetWindow().GetWidth(), GetWindow().GetHeight());
		//std::array<float, 16> view = , proj = ;
		
		//GW::MATH::GMatrix::LookAtLHF({ 0.f, 5.f, -5.5f }, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, reinterpret_cast<GW::MATH::GMATRIXF&>(view));
		//GW::MATH::GMatrix::ProjectionVulkanLHF(0.785398f, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), .1f, 1000.f, reinterpret_cast<GW::MATH::GMATRIXF&>(proj));
		Imgn::GBufferUBO gUBO
		{
			.viewMatrix = Imgn::Math::LookAtLH({ 0.f, 5.f, -5.5f }, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }),
			.projMatrix = cam.GetProjection()
		};

		Renderer().MapBufferData(Renderer().MakeBufferKey("G-BufferUBO", sizeof(Imgn::GBufferUBO)), &gUBO, sizeof(Imgn::GBufferUBO));

		//while (true)
		//{
		//	Renderer().StartFrame();
		//	Renderer().ExecuteGraph();
		//	Renderer().EndFrame();
		//}
	}

	~Daydream()
	{

	}
};

Imgn::ImgnApp* Imgn::CreateApplication()
{
	return new Daydream();
}