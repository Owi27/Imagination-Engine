#include "pch.hpp"
#include "ImgnCamera.h"

namespace Imgn
{
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

	void PerspectiveCamera::SetViewportSize(uint32_t pWidth, uint32_t pHeight)
	{
		if (pWidth == 0 || pHeight == 0) return;

		_aspect = static_cast<float>(pWidth) / static_cast<float>(pHeight);

		RecalculateProjection();
	}
}