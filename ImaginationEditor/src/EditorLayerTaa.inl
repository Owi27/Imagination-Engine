#pragma once
#include <cmath>
#include <array>

namespace Imgn::TaaObjectMv
{
	inline std::array<float, 12> PackPrevModel3x4(const mat4& m)
	{
		// Upper 3x4 of row-major mat4 -> HLSL float3x4 rows for mul(float3x4, float4).
		return {
			m[0], m[1], m[2], m[3],
			m[4], m[5], m[6], m[7],
			m[8], m[9], m[10], m[11]
		};
	}

	inline std::array<float, 12> ResolvePrevModel(TransformComponent* transform, const mat4& model)
	{
		const mat4 prev = transform->prevTransformValid ? transform->prevTransform : model;
		transform->prevTransform = model;
		transform->prevTransformValid = true;
		return PackPrevModel3x4(prev);
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
