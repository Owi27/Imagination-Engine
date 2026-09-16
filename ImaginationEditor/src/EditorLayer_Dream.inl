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

		TransformComponent* camXf = _sceneCamera->GetComponent<TransformComponent>();
		mat4 proj = _sceneCamera->GetComponent<CameraComponent>()->camera.GetProjection();
		vec2 jitter = GetProjectionJitter(_window->GetWidth(), _window->GetHeight());
		mat4 jitterMat = Math::Translate(Math::identity, { jitter[0] * JITTER_DEBUG_SCALE, jitter[1] * JITTER_DEBUG_SCALE, 0.f });
		const mat4 view = GetCamView(camXf);
		gBufferUBO.jitteredViewProj = view * (proj * jitterMat);
		gBufferUBO.viewProj = view * proj;

		const vec3 camForward = TaaObjectMv::CamForward(camXf);
		const vec3 camPos = camXf->position;
		if (_taaCamHistoryValid)
		{
			const float posDelta = Math::Length(camPos - _taaPrevCamPos);
			const float forwardDot = Math::Dot(camForward, _taaPrevCamForward);
			if (posDelta > kTaaCutPosThreshold || forwardDot < kTaaCutForwardDotMin)
				_taaInvalidateFrames = kTaaInvalidateFrameCount;
		}
		_taaPrevCamPos = camPos;
		_taaPrevCamForward = camForward;
		_taaCamHistoryValid = true;

		if (_taaInvalidateFrames > 0)
			_taaHistoryValid = false;

		_renderer->MapBufferData(gBufferUBOHandle, &gBufferUBO, sizeof(GBufferUBO));

		gBufferUBO.prevViewProj = gBufferUBO.viewProj;

		_renderer->ExecuteGraph();
		_renderer->CopyRenderImage("TAAResolved", "TAAHistory");

		if (_taaInvalidateFrames > 0)
		{
			--_taaInvalidateFrames;
		}
		else
		{
			_renderer->CopyRenderImage("G-BufferVelocity", "VelocityHistory");
			_taaHistoryValid = true;
		}

		_renderer->BlitToSwapchain("TAAResolved");
	}

	void EditorLayer::OnEvent(Event& pEvent)
	{
	}
}
