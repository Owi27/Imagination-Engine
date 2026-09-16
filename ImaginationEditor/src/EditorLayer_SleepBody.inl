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

