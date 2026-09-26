#pragma once
#include <Imgn.hpp>

namespace Imgn
{
    class EditorCamera : public ScriptableEntity
    {
		CameraComponent* cam = nullptr;
		TransformComponent* transform = nullptr;

		inline static bool _inputEnabled = false;

		void UpdateCamera(Time pTime);

	public:
		void Sleep()
		{
			IMGN_TRACE("sleep");
			cam = GetComponent<CameraComponent>();
			transform = GetComponent<TransformComponent>();
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