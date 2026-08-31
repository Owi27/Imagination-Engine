#pragma once
#include <Imgn.hpp>

namespace Imgn
{
    class EditorCamera : public ScriptableEntity
    {
		CameraComponent* cam;// = GetComponent<CameraComponent>();
		TransformComponent* transform;// = GetComponent<TransformComponent>();

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
	};
}