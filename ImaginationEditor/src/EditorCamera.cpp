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
		const float sensisitivty = .1f;
		float speed = cam->cameraSpeed * pTime;

		rotation[0] += std::clamp(deltaY * sensisitivty, -89.f, 89.f);
		rotation[1] += deltaX * sensisitivty;

		float pitch = Math::Radians(rotation[0]);
		float yaw = Math::Radians(rotation[1]);

		vec3 forward =
		{
			std::sin(yaw),
			0.f,
			std::cos(yaw)
		};

		vec3 right =
		{
			 std::cos(yaw),
			0.f,
			-std::sin(yaw)
		};

		float moveX = 0.f;
		float moveZ = 0.f;

		if (Imgn::Input::IsKeyPressed(IMGN_KEY_W)) moveZ += 1.f;
		else if (Imgn::Input::IsKeyPressed(IMGN_KEY_S)) moveZ -= 1.f;
		if (Imgn::Input::IsKeyPressed(IMGN_KEY_D)) moveX += 1.f;
		else if (Imgn::Input::IsKeyPressed(IMGN_KEY_A)) moveX -= 1.f;

		// local movement -> world movement
		position[0] += (right[0] * moveX + forward[0] * moveZ) * speed;
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

	mat4 EditorCamera::GetCamView()
	{
		const float pitch = Math::Radians(transform->rotation[0]);

		const float yaw =
			Math::Radians(transform->rotation[1]);

		vec3 forward =
		{
			std::sin(yaw) * std::cos(pitch),
			-std::sin(pitch),
			std::cos(yaw) * std::cos(pitch)
		};

		vec3 target =
		{
			transform->position[0] + forward[0],
			transform->position[1] + forward[1],
			transform->position[2] + forward[2]
		};

		return Math::LookAtLH(transform->position, target, { 0.f, 1.f, 0.f });
	}

}