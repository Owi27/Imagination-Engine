#pragma once
#include <cmath>

namespace Imgn::TaaObjectMv
{
	inline mat4 ResolvePrevModel(TransformComponent* transform, const mat4& model)
	{
		const mat4 prevModel = transform->prevTransformValid ? transform->prevTransform : model;
		transform->prevTransform = model;
		transform->prevTransformValid = true;
		return prevModel;
	}

	inline vec3 CamForward(const TransformComponent* camXf)
	{
		const float pitch = Math::Radians(camXf->rotation[0]);
		const float yaw = Math::Radians(camXf->rotation[1]);
		return {
			std::sin(yaw) * std::cos(pitch),
			-std::sin(pitch),
			std::cos(yaw) * std::cos(pitch)
		};
	}
}
