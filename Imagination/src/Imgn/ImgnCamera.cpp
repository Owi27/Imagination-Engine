#include "pch.hpp"
#include "ImgnCamera.h"

namespace Imgn
{
	//void PerspectiveCamera::RecalculateView()
	//{
	//	//_transform = Math::Translate(_transform, _pos);
	//	//	Math::Rotate(Math::identity, { _rot[0], _rot[1], _rot[2] }, _rot[3]);

	//	_view = Math::Inverse(_transform);
	//}

	void PerspectiveCamera::RecalculateProjection()
	{
		if (_farPlane <= _nearPlane) _farPlane = _nearPlane + 0.001f;

		_proj = Math::PerspectiveVKLH(_fov, _aspect, _nearPlane, _farPlane);
	}

	void PerspectiveCamera::SetFOV(float pFOV)
	{
		_fov = pFOV;

		RecalculateProjection();
	}

	void PerspectiveCamera::SetFarPlane(float pFarPlane)
	{
		_farPlane = pFarPlane;

		RecalculateProjection();
	}

	void PerspectiveCamera::SetNearPlane(float pNearPlane)
	{
		_nearPlane = pNearPlane;

		RecalculateProjection();
	}

	//void PerspectiveCamera::SetPosition(vec3 pPos)
	//{
	//	_pos = pPos;

	//	_transform[12] = _pos[0];
	//	_transform[13] = _pos[1];
	//	_transform[14] = _pos[2];
	//	_transform[15] = 1.f;

	//	RecalculateView();
	//}

	//void PerspectiveCamera::SetRotation(vec4 pRot)
	//{
	//	_rot = pRot;

	//	RecalculateView();
	//}

	//void PerspectiveCamera::SetViewportSize(uint32_t pWidth, uint32_t pHeight)
	//{
	//	if (pWidth == 0 || pHeight == 0) return;

	//	_aspect = static_cast<float>(pWidth) / static_cast<float>(pHeight);

	//	RecalculateProjection();
	//}

	//void PerspectiveCamera::Rotate(float pPitch, float pYaw)
	//{
	//	//_transform = Math::Translate(_transform, _pos);

	//	_transform = Math::Rotate(_transform, { 1.f, 0.f, 0.f }, pPitch);
	//	_transform = Math::Rotate(_transform, { 0.f, 1.f, 0.f }, pYaw, true);
	//	
	//	//for (size_t i = 0; i < 4; i++) _transform[i + 12] = row4[i];

	//	RecalculateView();
	//}

	void OrthographicCamera::RecalculateView()
	{
		mat4 transform = Math::Translate(Math::identity, _pos) * Math::Rotate(Math::identity, { _rot[0], _rot[1], _rot[2] }, _rot[3]);

		_view = Math::Inverse(transform);
	}

	void OrthographicCamera::RecalculateProjection()
	{
		_proj = Math::Orthographic(_right, _left, _top, _bottom);
	}

	void OrthographicCamera::SetFarPlane(float pFarPlane)
	{
		_farPlane = pFarPlane;

		RecalculateProjection();
	}

	void OrthographicCamera::SetNearPlane(float pNearPlane)
	{
		_nearPlane = pNearPlane;

		RecalculateProjection();
	}

	void OrthographicCamera::SetPosition(vec3 pPos)
	{
		_pos = pPos;

		RecalculateView();
	}

	void OrthographicCamera::SetRotation(vec4 pRot)
	{
		_rot = pRot;

		RecalculateView();
	}
	void Camera::RecalculateProjection()
	{
		if (type == CameraType::Perspective)
		{
			if (_farPlane <= _nearPlane) _farPlane = _nearPlane + 0.001f;

			_proj = Math::PerspectiveVKLH(_fov, _aspect, _nearPlane, _farPlane);
		}
		else
		{
			_proj = Math::Orthographic(_right * _aspect * .5f, _left * _aspect * .5f, _top * .5f, _bottom * .5f);
		}
	}
	void Camera::SetViewportSize(uint32_t pWidth, uint32_t pHeight)
	{
		if (pWidth == 0 || pHeight == 0) return;

		_aspect = static_cast<float>(pWidth) / static_cast<float>(pHeight);

		RecalculateProjection();
	}
}