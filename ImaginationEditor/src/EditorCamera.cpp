#include "pch.hpp"
#include "EditorCamera.h"

namespace Imgn
{
	void EditorCamera::UpdateCamera(Time pTime)
	{
		auto [deltaX, deltaY] = Input::GetMouseDelta();

		if (!Input::IsKeyPressed(IMGN_MOUSE_BUTTON_RIGHT)) return;

		vec3& position = transform->position;
		const float sensitivity = .1f;
		float speed = cam->cameraSpeed * pTime;

		_eulerRotation[0] = std::clamp(_eulerRotation[0] + deltaY * sensitivity, -89.f, 89.f);
		_eulerRotation[1] += deltaX * sensitivity;

		quat pitchRotation = Math::QuatFromEuler({ _eulerRotation[0], 0.f, 0.f });
		quat yawRotation = Math::QuatFromEuler({ 0.f, _eulerRotation[1], 0.f });

		transform->rotation = Math::Normalize(Math::Multiply(yawRotation, pitchRotation));

		float yaw = Math::Radians(_eulerRotation[1]);

		vec3 forward = { std::sin(yaw), 0.f, std::cos(yaw) };
		vec3 right = { std::cos(yaw), 0.f, -std::sin(yaw) };

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
	}

	mat4 EditorCamera::GetCamView()
	{
		vec3 forward = Math::Rotate(vec3{ 0.f, 0.f, 1.f }, transform->rotation);
		vec3 target = transform->position + forward;

		return Math::LookAtLH(transform->position, target, { 0.f, 1.f, 0.f });
	}
}