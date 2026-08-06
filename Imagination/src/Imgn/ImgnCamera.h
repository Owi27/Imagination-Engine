#pragma once

namespace Imgn
{
	class ICamera
	{
	public:
		virtual ~ICamera() = default;

		virtual void SetViewportSize(uint32_t pWidth, uint32_t pHeight) = 0;

		[[nodiscard]] virtual const mat4& GetProjection() const = 0;
	};

	class PerspectiveCamera : ICamera
	{
		float _fov = 45.f, _nearPlane = .1f, _farPlane = 1000.f, _aspect = 16.f / 9.f;
		vec3 _pos;
		mat4 _view, _proj;

		void RecalculateProjection();

	public:
		PerspectiveCamera(float pFOV = 45.0f, float pNearPlane = 0.1f, float pFarPlane = 1000.0f)
		{
			_fov = pFOV;
			_nearPlane = pNearPlane;
			_farPlane = pFarPlane;

			RecalculateProjection();
		}

		// Inherited via ICamera
		void SetFOV(float pFOV);
		void SetFarPlane(float pFarPlane);
		void SetNearPlane(float pNearPlane);
		void SetViewportSize(uint32_t pWidth, uint32_t pHeight) override;

		const mat4& GetProjection() const override { return _proj };
	};
}