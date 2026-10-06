#pragma once
#include <Imgn.hpp>

namespace Imgn
{
    class EditorCamera : public ScriptableEntity
    {
		CameraComponent* cam = nullptr;
		TransformComponent* transform = nullptr;

		vec3 _eulerRotation = { 0.f, 0.f, 0.f };
		inline static bool _inputEnabled = false;

		void UpdateCamera(Time pTime);

	public:
		void Sleep()
		{
			IMGN_TRACE("sleep");
			cam = GetComponent<CameraComponent>();
			transform = GetComponent<TransformComponent>();

			if (transform) _eulerRotation = Math::EulerFromQuat(transform->rotation);
		}

		void WakeUp()
		{

		}

		void Dream(Time pTime)
		{
			UpdateCamera(pTime);
		}

		mat4 GetCamView();

		static void SetInputEnabled(bool pEnabled) { _inputEnabled = pEnabled; }
	};
}