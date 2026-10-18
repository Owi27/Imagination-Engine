#include <Imgn.hpp>
#include "Panels/SceneHierarchy.h"
#include "Panels/ContentBrowser.hpp"
#include "Panels/ExternalAppPanel.h"


namespace Imgn
{
    class EditorLayer : public Layer
    {
		enum class eSceneState
		{
			Edit,
			Play,
			Pause,
			Simulate
		} _sceneState = eSceneState::Edit, _requestedSceneState = eSceneState::Edit;

        ImgnWindow* _window;
        ImgnRenderer* _renderer;
        vk::DescriptorSet _sceneWindow = nullptr, _playIconDS = nullptr, _stopIconDS = nullptr;

		float _cameraSpeed = 500.f, _yaw = 0.f; // , _sceneViewportAspect = 0.f;
		//vec3 _camPos = { 0, 0, 0 };
		//PerspectiveCamera _camera;
		bool _cameraLookActive = false;
		std::array<uint32_t, MaxFramesInFlight> gBufferUBOHandles;
		uint32_t _sceneWidth = 0, _sceneHeight = 0, _pointLightBuffer;

		ID _selectionBeforePlay = 0;
		shared<Scene> _activeScene, _pendingScene, _runtimeScene, _sceneBeforePlay;
		Entity* _sceneCamera = nullptr;
		Entity* _selectedEntity = nullptr;

		unique<Image> _playIcon, _stopIcon;

		uint64_t _pointLightBufferSize = 0;

		static constexpr std::array<vec3, 6> PointShadowDirections =
		{
			vec3{ 1.f, 0.f, 0.f },
			vec3{ -1.f, 0.f, 0.f },
			vec3{ 0.f, 1.f, 0.f },
			vec3{ 0.f, -1.f, 0.f },
			vec3{ 0.f, 0.f, 1.f },
			vec3{ 0.f, 0.f, -1.f }
		};

		static constexpr std::array<vec3, 6> PointShadowUp =
		{
			vec3{ 0.f, -1.f, 0.f },
			vec3{ 0.f, -1.f, 0.f },
			vec3{ 0.f, 0.f, 1.f },
			vec3{ 0.f, 0.f, -1.f },
			vec3{ 0.f, -1.f, 0.f },
			vec3{ 0.f, -1.f, 0.f }
		};

		std::array<std::vector<uint32_t>, MaxFramesInFlight> _pointShadowUBOHandles;
		std::vector<mat4> _pointViewProjs;

		ID _hoveredEntityID = 0;
		Entity* _hoveredEntity = nullptr;

		//panels
		SceneHierarchy _sceneHierarchy;
		ContentBrowser _contentBrowser;
		AppPanel blenderPanel;

		//ubo
		GBufferUBO gBufferUBO;

		bool _taaHistoryValid = false;

		std::default_random_engine gen = std::default_random_engine(777);

		uint32_t _renderWidth = 0, _renderHeight = 0;
		uint32_t _jitterFrameIndex = 0;

		shared<Scene> _editorScene;

		int _gizmoType = -1;

		void ResizeSceneTargets();
		void DrawSceneView();

		mat4 GetCamView(TransformComponent* pTransform);

		//uint32_t _jitterFrameIndex = 0;
		vec2 GetJitterSample();
		vec2 GetProjectionJitter(uint32_t pWidth, uint32_t pHeight);

		void ApplySceneState();
		void ApplyPendingScene();

		void UIToolbar();
		void ScenePlay();
		void SceneStop();
		void SetActiveScene(shared<Scene> pScene, ID pSelectedEntityID = 0);

		//shortcuts
		void NewScene();
		void OpenScene();
		void SaveScene();
		void SaveSceneAs();

	public:
        EditorLayer() /*Constructor*/
        {
        }

        EditorLayer(ImgnWindow* pWindow, ImgnRenderer* pRenderer) /*Constructor*/
        {
            _renderer = pRenderer;
            _window = pWindow;
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