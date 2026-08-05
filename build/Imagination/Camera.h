#pragma once

namespace Imgn
{
	class PerspectiveCamera
	{
		float _fov = 45.f, _nearPlane = .1f, _farPlane = 1000.f, _aspect = 16.f / 9.f;
		vec3 _pos;
		mat4 _view, _proj;

	public:
		PerspectiveCamera();
		~PerspectiveCamera();

	private:

	};
}