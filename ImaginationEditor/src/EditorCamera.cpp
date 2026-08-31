#include "pch.hpp"
#include "EditorCamera.h"

namespace Imgn
{
	void EditorCamera::UpdateCamera(Time pTime)
	{
		auto [deltaX, deltaY] = Input::GetMouseDelta();

		if (!Input::IsKeyPressed(IMGN_MOUSE_BUTTON_RIGHT)) return;

		vec3& position = transform->position;
		vec3& rotation = transform->rotation;
		float speed = cam->cameraSpeed * pTime;

		float pitch = Imgn::Math::Radians(45.f) * deltaY / cam->camera.GetHeight();
		float yaw = Imgn::Math::Radians(45.f) * cam->camera.GetAspect() * deltaX / cam->camera.GetWidth();

		rotation[0] += pitch;
		rotation[1] += yaw;

		vec3 forward =
		{
			std::sin(Math::Radians(rotation[1])) * std::cos(Math::Radians(rotation[0])),
			-std::sin(Math::Radians(rotation[0])),
			std::cos(Math::Radians(rotation[1])) * std::cos(Math::Radians(rotation[0]))
		};

		vec3 right =
		{
			std::cos(Math::Radians(rotation[1])),
			0.0f,
			-std::sin(Math::Radians(rotation[1]))
		};

		float moveX = 0.f;
		float moveZ = 0.f;

		if (Imgn::Input::IsKeyPressed(IMGN_KEY_W)) moveZ += 1.f;
		else if (Imgn::Input::IsKeyPressed(IMGN_KEY_S)) moveZ -= 1.f;
		if (Imgn::Input::IsKeyPressed(IMGN_KEY_D)) moveX += 1.f;
		else if (Imgn::Input::IsKeyPressed(IMGN_KEY_A)) moveX -= 1.f;

		// local movement -> world movement
		position[0] += (right[0] * moveX + forward[0] * moveZ) * speed;
		position[1] += (right[1] * moveX + forward[1] * moveZ) * speed;
		position[2] += (right[2] * moveX + forward[2] * moveZ) * speed;

		if (Imgn::Input::IsKeyPressed(IMGN_KEY_SPACE))
		{
			if (Imgn::Input::IsKeyPressed(IMGN_KEY_LEFT_SHIFT)) position[1] -= speed;
			else position[1] += speed;
		}

		//transform->position = camPos;

		//rotation
		//auto [deltaX, deltaY] = Imgn::Input::GetMouseDelta();
		//	//_transform = Math::Translate(_transform, _pos);
		//transform->rotation[0] += Math::Degrees(pitch);
		//transform->rotation[1] += Math::Degrees(yaw);
	}

}