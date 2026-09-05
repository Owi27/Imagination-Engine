#include <Imgn.hpp>
#include "Panels/SceneHierarchy.h"

namespace Imgn
{
    class EditorLayer : public Layer
    {
        ImgnWindow* _window;
        ImgnRenderer* _renderer;
        vk::DescriptorSet _sceneWindow = nullptr;

		float _cameraSpeed = 500.f, _yaw = 0.f; // , _sceneViewportAspect = 0.f;
		//vec3 _camPos = { 0, 0, 0 };
		//PerspectiveCamera _camera;
		bool _cameraLookActive = false;
		uint32_t gBufferUBOHandle = 0, _sceneWidth = 0, _sceneHeight = 0;

		shared<Scene> _activeScene;
		Entity* _sceneCamera = nullptr;
		Entity* _selectedEntity = nullptr;
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

		//panels
		SceneHierarchy _sceneHierarchy;

		//ubo
		GBufferUBO gBufferUBO;

		std::default_random_engine gen = std::default_random_engine(777);

		mat4 GetCamView(TransformComponent* pTransform);

		//uint32_t _jitterFrameIndex = 0;
		vec2 GetJitterSample();
		vec2 GetProjectionJitter(uint32_t pWidth, uint32_t pHeight);

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