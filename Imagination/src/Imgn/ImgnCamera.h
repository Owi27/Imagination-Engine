#pragma once
#include "ImgnMath.h"

namespace Imgn
{
	class ICamera
	{
	public:
		virtual ~ICamera() = default;
		
		virtual void SetPosition(vec3 pPos) = 0;
		virtual void SetRotation(vec4 pRot) = 0;
		//virtual void SetViewportSize(uint32_t pWidth, uint32_t pHeight) = 0;

		virtual void RecalculateView() = 0;
		virtual void RecalculateProjection() = 0;

		[[nodiscard]] virtual const mat4& GetView() const = 0;
		[[nodiscard]] virtual const mat4 GetViewProj() const = 0;
		[[nodiscard]] virtual const mat4& GetProjection() const = 0;
	};

	enum class CameraType : uint8_t
	{
		Perspective, Orthographic
	};

	class IMGN_API Camera
	{
		mat4 _proj = Math::identity;
		CameraType _type = CameraType::Perspective;
		uint32_t _width = 0, _height = 0;
		float _fov = Math::Radians(45), _nearPlane = .1f, _farPlane = 5000.f, _aspect = 16.f / 9.f, _orthoSize = 1.f, _orthoNear = -1.f, _orthoFar = 1.f;

	public:
		Camera()
		{
		}

		~Camera() /*Destructor*/
		{
		}

		/*Copy Constructor*/
		Camera(const Camera& pOther) = default;

		/*Copy Assignment Operator*/
		Camera& operator=(const Camera& pOther) = default;

		/*Move Constructor*/
		Camera(Camera&& pOther) noexcept = default;

		/*Move Assignment Operator*/
		Camera& operator=(Camera&& pOther) noexcept = default;

		void CreatePerspectiveCamera(float pFOV = Math::Radians(45.f), float pNearPlane = .1f, float pFarPlane = 5000.f)
		{
			_type = CameraType::Perspective;

			_fov = pFOV;
			_nearPlane = pNearPlane;
			_farPlane = pFarPlane;

			RecalculateProjection();
		}

		void CreateOrthographicCamera(float pOrthoSize, float pNear = -1.f, float pFar = 1.f)
		{
			_type = CameraType::Orthographic;

			_orthoSize = pOrthoSize;
			_orthoNear = pNear;
			_orthoFar = pFar;

			RecalculateProjection();
		}

		/*Class Functions*/
		void RecalculateProjection();
		const mat4& GetProjection() const { return _proj; };
		void SetViewportSize(uint32_t pWidth, uint32_t pHeight);
		
		uint32_t& GetWidth() { return _width; }
		uint32_t& GetHeight() { return _height; }

		CameraType GetType() { return _type; }
		void SetType(CameraType pType) { _type = pType; }

		//pers
		float& GetFOV() { return _fov; }
		float& GetAspect() { return _aspect; }
		float& GetFarPlane() { return _farPlane; }
		float& GetNearPlane() { return _nearPlane; }

		//ortho
		float& GetOrthoSize() { return _orthoSize; }
		float& GetOrthoNear() { return _orthoNear; }
		float& GetOrthoFar() { return _orthoFar; }
		//float& GetOrthoBottom() { return _bottom; }
	};

	class IMGN_API PerspectiveCamera : ICamera
	{
		float _fov = Math::Radians(45), _nearPlane = .1f, _farPlane = 1000.f, _aspect = 16.f / 9.f;

		vec3 _pos = { 0.f, 0.f, 0.f };
		vec4 _rot = { 0.f, 0.f, 0.f, 0.f };
		mat4 _transform = Math::identity, _view = Math::identity, _proj = Math::identity;

	public:
		PerspectiveCamera(float pFOV = Math::Radians(45.f), float pNearPlane = .1f, float pFarPlane = 1000.f)
		{
			_fov = pFOV;
			_nearPlane = pNearPlane;
			_farPlane = pFarPlane;

			RecalculateProjection();
		}

		// Inherited via ICamera
		const mat4& GetProjection() const override { return _proj; };
		//const mat4& GetView() const override { return _view; }
		//const mat4 GetViewProj() const override { return _view * _proj; }
		//const vec3& GetPosition() const { return _pos; }

		//void RecalculateView() override;
		void RecalculateProjection() override;

		void SetFOV(float pFOV);
		void SetFarPlane(float pFarPlane);
		void SetNearPlane(float pNearPlane);
		//void SetPosition(vec3 pPos) override;
		//void SetRotation(vec4 pRot) override;
		//void SetViewportSize(uint32_t pWidth, uint32_t pHeight);// override;

		//void Rotate(float pPitch, float pYaw);
	};

	class IMGN_API OrthographicCamera : ICamera
	{
		float _right = 1.f, _left = 1.f, _top = 1.f, _bottom = 1.f, _nearPlane = -1.f, _farPlane = 1.f, _aspect = 16.f / 9.f;
		vec3 _pos = { 0.f, 0.f, 0.f };
		vec4 _rot = { 0.f, 0.f, 0.f, 0.f };
		mat4 _view, _proj;

	public:
		OrthographicCamera(float pRight, float pLeft, float pTop, float pBottom, float pNear = -1.f, float pFar = 1.f)
		{
			_right = pRight;
			_left = pLeft;
			_top = pTop;
			_bottom = pBottom;

			RecalculateProjection();
		}

		// Inherited via ICamera
		//const mat4& GetView() const override { return _view; }
		const mat4& GetProjection() const override { return _proj; };
		const mat4 GetViewProj() const override { return _view * _proj; }

		void RecalculateView() override;
		void RecalculateProjection() override;

		//void SetFOV(float pFOV);
		void SetFarPlane(float pFarPlane);
		void SetNearPlane(float pNearPlane);
		void SetPosition(vec3 pPos) override;
		void SetRotation(vec4 pRot) override;
		//void SetViewportSize(uint32_t pWidth, uint32_t pHeight) override;
	};
}