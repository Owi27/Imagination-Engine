#include <Imgn.hpp>

class TestLayer : public Imgn::Layer
{
	float _cameraSpeed = 100.f, _yaw = 0.f;
	vec3 _camPos = { 0, 0, 0 };
	Imgn::PerspectiveCamera _camera;

	Imgn::ImgnWindow* _window;
	Imgn::ImgnRenderer* _renderer;

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

public:
	TestLayer() : Layer("Test")
	{
	}

	TestLayer(Imgn::ImgnWindow* pWindow, Imgn::ImgnRenderer* pRenderer) : Layer("Test")
	{
		_renderer = pRenderer;
		_window = pWindow;
		_camera.SetFarPlane(10000.f);
		_camera.SetViewportSize(_window->GetWidth(), _window->GetHeight());
		Imgn::ImgnGLTF gltf;
		Imgn::ImgnModel sponza = gltf.LoadModel("../../Models/Sponza/glTF/Sponza.gltf", *_renderer);

		Imgn::GBufferUBO gBufferUBO
		{
			.viewProj = _camera.GetViewProj()
		};

		gBufferUBOHandle = _renderer->CreateUniformBuffer(&gBufferUBO, sizeof(Imgn::GBufferUBO));

		Imgn::RenderPass gBuffer
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
						.range = sizeof(Imgn::GBufferUBO)
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
						Imgn::GBufferPC pc
						{
							.model = Imgn::Math::identity,
							.materialIndex = prim.material
						};

						commandBuffer.pushConstants<Imgn::GBufferPC>(*_renderer->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, pc);
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

				Imgn::LightingPC pc
				{
					.invViewProj = Imgn::Math::Inverse(_camera.GetViewProj()),
					.camPos = _camera.GetPosition(),
					.width = _window->GetWidth(),
					.height = _window->GetHeight()
				};

				commandBuffer.pushConstants<Imgn::LightingPC>(*_renderer->GetPipelineLayout(), vk::ShaderStageFlagBits::eCompute, 0, pc);

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

		//Imgn::PerspectiveCamera cam;
		//cam.SetViewportSize(GetWindow().GetWidth(), GetWindow().GetHeight());
		//std::array<float, 16> view = , proj = ;

		//GW::MATH::GMatrix::LookAtLHF({ 0.f, 5.f, -5.5f }, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, reinterpret_cast<GW::MATH::GMATRIXF&>(view));
		//GW::MATH::GMatrix::ProjectionVulkanLHF(0.785398f, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), .1f, 1000.f, reinterpret_cast<GW::MATH::GMATRIXF&>(proj));

		//Imgn::GBufferUBO
		//{
		//	_
		//}
		//_renderer->MapBufferData("G-BufferUBO", &gUBO, sizeof(Imgn::GBufferUBO));
	}

	void Dream(Imgn::Time pTime) override
	{
		UpdateCamera(pTime);

		Imgn::GBufferUBO gBufferUBO
		{
			.viewProj = _camera.GetViewProj()
		};
		_renderer->MapBufferData(gBufferUBOHandle, &gBufferUBO, sizeof(Imgn::GBufferUBO));
		//IMGN_INFO("DeltaTime {}s : {}ms", pTime.Seconds(), pTime.MiliSeconds());

		if (Imgn::Input::IsKeyPressed(IMGN_KEY_TAB))
		{
			IMGN_INFO("Tab key pressed");
		}

		_renderer->ExecuteGraph();
		_renderer->BlitToSwapchain("LitScene");
	}

	void OnEvent(Event& pEvent) override
	{
		EventDispatcher dispatcher(pEvent);
		dispatcher.Dispatch<KeyPressedEvent>(IMGN_BIND_EVENT_FN(TestLayer::OnKeyPressedEvent));
	}

	bool OnKeyPressedEvent(KeyPressedEvent& pEvent)
	{
		/*if (pEvent.GetKeyCode() == IMGN_KEY_W)
			_camPos[2] += _cameraSpeed;
		if (pEvent.GetKeyCode() == IMGN_KEY_A)
			_camPos[0] -= _cameraSpeed;
		if (pEvent.GetKeyCode() == IMGN_KEY_S)
			_camPos[2] -= _cameraSpeed;
		if (pEvent.GetKeyCode() == IMGN_KEY_D)
			_camPos[0] += _cameraSpeed;*/

		return false;
	}

	//Imgn::PerspectiveCamera& GetCamera() { return _camera; }
	/* Class Functions */
	void UpdateCamera(Imgn::Time pTime)
	{
		float speed = _cameraSpeed * pTime;
		float moveX = 0.f;
		float moveZ = 0.f;

		if (Imgn::Input::IsKeyPressed(IMGN_KEY_W)) moveZ += 1.f;
		else if (Imgn::Input::IsKeyPressed(IMGN_KEY_S)) moveZ -= 1.f;
		if (Imgn::Input::IsKeyPressed(IMGN_KEY_D)) moveX += 1.f;
		else if (Imgn::Input::IsKeyPressed(IMGN_KEY_A)) moveX -= 1.f;

		// local movement -> world movement
		_camPos[0] += (moveX * std::cos(_yaw) + moveZ * std::sin(_yaw)) * speed;
		_camPos[2] += (-moveX * std::sin(_yaw) + moveZ * std::cos(_yaw)) * speed;

		if (Imgn::Input::IsKeyPressed(IMGN_KEY_SPACE))
		{
			if (Imgn::Input::IsKeyPressed(IMGN_KEY_LEFT_SHIFT)) _camPos[1] -= _cameraSpeed * pTime;
			else _camPos[1] += _cameraSpeed * pTime;
		}

		// local movement -> world movement
		_camPos[0] += (moveX * std::cos(_yaw) + moveZ * std::sin(_yaw)) * pTime;
		_camPos[2] += (-moveX * std::sin(_yaw) + moveZ * std::cos(_yaw)) * pTime;

		_camera.SetPosition(_camPos);

		//rotation
		auto [deltaX, deltaY] = Imgn::Input::GetMouseDelta();
		float pitch = Imgn::Math::Radians(45.f) * deltaY / _window->GetHeight();
		float yaw = Imgn::Math::Radians(45.f) * _window->GetAspectRatio() * deltaX / _window->GetWidth();

		_yaw += yaw;

		_camera.Rotate(pitch, yaw);
	}
};

class Daydream : public Imgn::ImgnApp
{
public:
	Daydream()
	{
		AddLayer(Unique<TestLayer>(&GetWindow(), &Renderer())); //treating as my rendering layer for now
		AddOverlay(Unique<Imgn::ImGuiLayer>(&GetWindow(), &Renderer()));
		//Imgn::ImgnGLTF gltf;
		//Imgn::ImgnModel sponza = gltf.LoadModel("../../Models/Sponza/glTF/Sponza.gltf", Renderer());

		//std::vector vertices =
		//{
		//	Vertex
		//	{
		//		.pos = { 0.f, .5f, 0.f}
		//	},
		//	Vertex
		//	{
		//		.pos = { .5f, -.5f, 0.f}
		//	},
		//	Vertex
		//	{
		//		.pos = { -.5f, -.5f, 0.f}
		//	}
		//};

		//uint32_t vertexBuffer = Renderer().CreateVertexBuffer(vertices);

		//Imgn::RenderPass gBuffer
		//{
		//	.name = "G-BufferPass",
		//	.imageOUT =
		//	{
		//		Renderer().MakeImageKey("G-BufferAlbedo", GetWindow().GetWidth(), GetWindow().GetHeight()),
		//		Renderer().MakeImageKey("G-BufferNormal", GetWindow().GetWidth(), GetWindow().GetHeight()),
		//		Renderer().MakeImageKey("G-BufferMaterial", GetWindow().GetWidth(), GetWindow().GetHeight()),
		//		Renderer().MakeImageKey("G-BufferEmissive", GetWindow().GetWidth(), GetWindow().GetHeight()),
		//		Renderer().MakeImageKey("Depth", GetWindow().GetWidth(), GetWindow().GetHeight())
		//	},
		//	.bufferOUT =
		//	{
		//		Renderer().MakeBufferKey("G-BufferUBO", sizeof(Imgn::GBufferUBO)), //edit size
		//	},
		//	.Execute = [&, sponza](vk::raii::CommandBuffer& commandBuffer)
		//	{
		//		std::array colorAttachments =
		//		{
		//			vk::RenderingAttachmentInfo
		//			{
		//				.imageView = *Renderer().GetRenderGraphImage(Renderer().MakeImageKey("G-BufferAlbedo", GetWindow().GetWidth(), GetWindow().GetHeight())).image.view,
		//				.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		//				.loadOp = vk::AttachmentLoadOp::eClear,
		//				.storeOp = vk::AttachmentStoreOp::eStore,
		//				.clearValue = vk::ClearColorValue{ std::array<float, 4>{1.0f, 0.25f, 0.75f, 1.0f} },
		//			},
		//			vk::RenderingAttachmentInfo
		//			{
		//				.imageView = *Renderer().GetRenderGraphImage(Renderer().MakeImageKey("G-BufferNormal", GetWindow().GetWidth(), GetWindow().GetHeight())).image.view,
		//				.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		//				.loadOp = vk::AttachmentLoadOp::eClear,
		//				.storeOp = vk::AttachmentStoreOp::eStore,
		//				.clearValue = vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} },
		//			},
		//			vk::RenderingAttachmentInfo
		//			{
		//				.imageView = *Renderer().GetRenderGraphImage(Renderer().MakeImageKey("G-BufferMaterial", GetWindow().GetWidth(), GetWindow().GetHeight())).image.view,
		//				.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		//				.loadOp = vk::AttachmentLoadOp::eClear,
		//				.storeOp = vk::AttachmentStoreOp::eStore,
		//				.clearValue = vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} },
		//			},
		//			vk::RenderingAttachmentInfo
		//			{
		//				.imageView = *Renderer().GetRenderGraphImage(Renderer().MakeImageKey("G-BufferEmissive", GetWindow().GetWidth(), GetWindow().GetHeight())).image.view,
		//				.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		//				.loadOp = vk::AttachmentLoadOp::eClear,
		//				.storeOp = vk::AttachmentStoreOp::eStore,
		//				.clearValue = vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} },
		//			},
		//		};

		//		vk::RenderingAttachmentInfo depthAttachment
		//		{
		//			.imageView = *Renderer().GetRenderGraphImage(Renderer().MakeImageKey("Depth", GetWindow().GetWidth(), GetWindow().GetHeight())).image.view,
		//			.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
		//			.loadOp = vk::AttachmentLoadOp::eClear,
		//			.storeOp = vk::AttachmentStoreOp::eStore,
		//			.clearValue = vk::ClearDepthStencilValue{ .0f, 0 },
		//		};

		//		vk::RenderingInfo renderingInfo
		//		{
		//			.renderArea = { {0, 0}, { GetWindow().GetWidth(), GetWindow().GetHeight() } },
		//			.layerCount = 1,
		//			.colorAttachmentCount = colorAttachments.size(),
		//			.pColorAttachments = colorAttachments.data(),
		//			.pDepthAttachment = &depthAttachment,
		//		};

		//		commandBuffer.beginRendering(renderingInfo);
		//		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, Renderer().GetGBufferPipeline());
		//		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, Renderer().GetPipelineLayout(), 1, *Renderer().GetTextureDescriptorSet(), {});
		//		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0, static_cast<float>(GetWindow().GetWidth()), static_cast<float>(GetWindow().GetHeight()), 0.0f, 1.0f));
		//		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), { static_cast<float>(GetWindow().GetWidth()), static_cast<float>(GetWindow().GetHeight()) }));
		//		//push descriptor set
		//		{
		//			//uniform buffer
		//			vk::DescriptorBufferInfo uboInfo
		//			{
		//				.buffer = *Renderer().GetRenderGraphBuffer(Renderer().MakeBufferKey("G-BufferUBO", sizeof(Imgn::GBufferUBO))).buffer.buffer,
		//				.offset = 0,
		//				.range = sizeof(Imgn::GBufferUBO)
		//			};
		//			
		//			vk::DescriptorBufferInfo materialSBInfo
		//			{
		//				.buffer = *Renderer().GetBuffer(sponza.materialBuffer).buffer,
		//				.offset = 0,
		//				.range = sponza.materialBufferSize
		//			};

		//			const std::array writes
		//			{
		//				vk::WriteDescriptorSet
		//				{
		//					.dstSet = nullptr,
		//					.dstBinding = 0,
		//					.dstArrayElement = 0,
		//					.descriptorCount = 1,
		//					.descriptorType = vk::DescriptorType::eUniformBuffer,
		//					.pBufferInfo = &uboInfo
		//				},
		//				vk::WriteDescriptorSet
		//				{
		//					.dstSet = nullptr,
		//					.dstBinding = 1,
		//					.dstArrayElement = 0,
		//					.descriptorCount = 1,
		//					.descriptorType = vk::DescriptorType::eStorageBuffer,
		//					.pBufferInfo = &materialSBInfo
		//				},
		//			};

		//			commandBuffer.pushDescriptorSet(vk::PipelineBindPoint::eGraphics, Renderer().GetPipelineLayout(), 0, writes);
		//		}
		//		
		//		for (uint32_t meshHandle : sponza.meshes)
		//		{
		//			commandBuffer.bindVertexBuffers(0, **Renderer().GetBuffer(Renderer().GetMesh(meshHandle).vertexBuffer).buffer, {0});
		//			commandBuffer.bindIndexBuffer(**Renderer().GetBuffer(Renderer().GetMesh(meshHandle).indexBuffer).buffer, 0, vk::IndexType::eUint32);

		//			for (auto& prim : Renderer().GetMesh(meshHandle).primitives)
		//			{
		//				Imgn::GBufferPC pc
		//				{
		//					.model = std::bit_cast<std::array<float, 16>>(GW::MATH::GIdentityMatrixF),
		//					.materialIndex = prim.material
		//				};

		//				commandBuffer.pushConstants<Imgn::GBufferPC>(*Renderer().GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, pc);
		//				commandBuffer.drawIndexed(prim.indexCount, 1, prim.firstIndex, prim.vertexOffset, 0);
		//			}
		//		}

		//		commandBuffer.endRendering();
		//	}
		//};

		//Imgn::RenderPass lighting
		//{
		//	.name = "LightingPass",
		//	.imageIN =
		//	{
		//		Renderer().MakeImageKey("G-BufferAlbedo", GetWindow().GetWidth(), GetWindow().GetHeight()),
		//		Renderer().MakeImageKey("G-BufferNormal", GetWindow().GetWidth(), GetWindow().GetHeight()),
		//		Renderer().MakeImageKey("G-BufferMaterial", GetWindow().GetWidth(), GetWindow().GetHeight()),
		//		Renderer().MakeImageKey("G-BufferEmissive", GetWindow().GetWidth(), GetWindow().GetHeight()),
		//		Renderer().MakeImageKey("Depth", GetWindow().GetWidth(), GetWindow().GetHeight())
		//	},
		//	.imageOUT =
		//	{
		//		Renderer().MakeImageKey("LitScene", GetWindow().GetWidth(), GetWindow().GetHeight()),
		//	},
		//	.bufferOUT =
		//	{
		//		Renderer().MakeBufferKey("LightingUBO", 256), //edit size
		//		Renderer().MakeBufferKey("LightingSBO", 256)
		//	},
		//	.Execute = [&](vk::raii::CommandBuffer& commandBuffer)
		//	{
		//		/*vk::RenderingAttachmentInfo colorAttachment
		//		{
		//			.imageView = *Renderer().GetRenderGraphImage(Renderer().MakeImageKey("LitScene", GetWindow().GetWidth(), GetWindow().GetHeight())).image.view,
		//			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		//			.loadOp = vk::AttachmentLoadOp::eClear,
		//			.storeOp = vk::AttachmentStoreOp::eStore,
		//			.clearValue = vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} },
		//		};

		//		vk::RenderingAttachmentInfo depthAttachment
		//		{
		//			.imageView = *Renderer().GetRenderGraphImage(Renderer().MakeImageKey("Depth", GetWindow().GetWidth(), GetWindow().GetHeight())).image.view,
		//			.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
		//			.loadOp = vk::AttachmentLoadOp::eClear,
		//			.storeOp = vk::AttachmentStoreOp::eStore,
		//			.clearValue = vk::ClearDepthStencilValue{ 1.0f, 0 },
		//		};

		//		vk::RenderingInfo renderingInfo
		//		{
		//			.renderArea = { {0, 0}, { GetWindow().GetWidth(), GetWindow().GetHeight() } },
		//			.layerCount = 1,
		//			.colorAttachmentCount = 1,
		//			.pColorAttachments = &colorAttachment,
		//			.pDepthAttachment = &depthAttachment,
		//		};

		//		commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, Renderer().GetLightingPipeline());*/
		//		
		//		//push descriptor set
		//		{
		//			//uniform buffer
		//			/*vk::DescriptorBufferInfo uboInfo
		//			{
		//				.buffer = *Renderer().GetRenderGraphBuffer("G-BufferUBO").buffer.buffer,
		//				.offset = 0,
		//				.range = 192
		//			};

		//			const std::array writes
		//			{
		//				vk::WriteDescriptorSet
		//				{
		//					.dstSet = nullptr,
		//					.dstBinding = 0,
		//					.dstArrayElement = 0,
		//					.descriptorCount = 1,
		//					.descriptorType = vk::DescriptorType::eUniformBuffer,
		//					.pBufferInfo = &uboInfo
		//				},
		//			};

		//			commandBuffer.pushDescriptorSet(vk::PipelineBindPoint::eGraphics, Renderer().GetPipelineLayout(), 0, writes);*/
		//		}

		//		//commandBuffer.dispatch(std::ceil(GetWindow().GetWidth() / 8), std::ceil(GetWindow().GetHeight() / 8), 1);
		//	}
		//};


		//Renderer().AddPass(gBuffer);
		//Renderer().AddPass(lighting);
		//Renderer().CompileGraph();

		//Imgn::PerspectiveCamera cam;
		//cam.SetViewportSize(GetWindow().GetWidth(), GetWindow().GetHeight());
		////std::array<float, 16> view = , proj = ;
		//
		////GW::MATH::GMatrix::LookAtLHF({ 0.f, 5.f, -5.5f }, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, reinterpret_cast<GW::MATH::GMATRIXF&>(view));
		////GW::MATH::GMatrix::ProjectionVulkanLHF(0.785398f, static_cast<float>(GetWindow().GetWidth()) / static_cast<float>(GetWindow().GetHeight()), .1f, 1000.f, reinterpret_cast<GW::MATH::GMATRIXF&>(proj));
		//Imgn::GBufferUBO gUBO
		//{
		//	.viewProj = cam.GetViewProj(),//Imgn::Math::LookAtLH({ 0.f, 5.f, -5.5f }, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }),
		//	//.projMatrix = cam.GetProjection()
		//};

		//Renderer().MapBufferData(Renderer().MakeBufferKey("G-BufferUBO", sizeof(Imgn::GBufferUBO)), &gUBO, sizeof(Imgn::GBufferUBO));

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