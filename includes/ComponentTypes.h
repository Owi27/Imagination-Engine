#pragma once

#include "ResourceTypes.h"
#include "Events/Event.h"

//class TransformComponent : public Component
//{
//	mutable Math::mat4<float> _transform;
//	mutable bool _isTransformDirty = true;
//
//	Math::vec3<float> _position, _rotation, _scale;
//
//public:
//	void SetPosition(const Math::vec3<float>& pPos)
//	{
//		_position = pPos;
//		_isTransformDirty = true;
//	}
//
//	void SetRotation(const Math::vec3<float>& pRot)
//	{
//		_rotation = pRot;
//		_isTransformDirty = true;
//	}
//
//	void SetScale(const Math::vec3<float>& pS)
//	{
//		_scale = pS;
//		_isTransformDirty = true;
//	}
//
//	const Math::vec3<float>& GetPosition() const { return _position; }
//	const Math::vec3<float>& GetRotation() const { return _rotation; }
//	const Math::vec3<float>& GetScale() const { return _scale; }
//
//	Math::mat4<float> GetTransformMatrix() const
//	{
//		if (_isTransformDirty)
//		{
//			//// Calculate transformation matrix
//			//glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
//			//glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
//			//glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
//
//			//transformMatrix = translationMatrix * rotationMatrix * scaleMatrix;
//			//transformDirty = false;
//		}
//
//		return _transform;
//	}
//};
//
//struct Material
//{
//
//};
//
//class MeshComponent : public Component
//{
//	Mesh* _mesh = nullptr;
//	Material* _material = nullptr;
//
//public:
//	MeshComponent(Mesh* pMesh, Material* pMaterial)
//	{
//		_mesh = pMesh;
//		_material = pMaterial;
//	}
//
//	void SetMesh(Mesh* pMesh) { _mesh = pMesh; }
//	void SetMaterial(Material* pMaterial) { _material = pMaterial; }
//
//	Mesh* GetMesh() const { return _mesh; }
//	Material* GetMaterial() const { return _material; }
//
//	void Render() override
//	{
//		if (!_mesh || !_material) return;
//
//		// Get transform component
//		auto transform = GetOwner()->GetComponent<TransformComponent>();
//		if (!transform) return;
//
//		// Render mesh with material and transform
//		//_material->Bind();
//		//_material->SetUniform("modelMatrix", transform->GetTransformMatrix());
//		//_mesh->Render();
//	}
//};
//
//class CameraComponent : public Component, public EventListener
//{
//	float _fov = 45.f, _aspect = 1.77777777778/*16.f/9.f*/, _nearPlane = .1f, _farPlane = 1000.f;
//
//	mutable Math::mat4<float> _view = Math::Identity<float>(), _proj = Math::Identity<float>();
//
//	mutable bool _isProjectionDirty = true;
//
//public:
//	void SetPerspective(float pFov, float pAspect, float pNear, float pFar)
//	{
//		_fov = pFov;
//		_aspect = pAspect;
//		_nearPlane = pNear;
//		_farPlane = pFar;
//		_isProjectionDirty = true;
//	}
//
//	Math::mat4<float> GetViewMatrix() const
//	{
//		// Get transform component
//		auto transform = GetOwner()->GetComponent<TransformComponent>();
//
//		if (transform)
//		{
//			//// Calculate view matrix from transform
//			//Math::vec3 position = transform->GetPosition();
//			//Math::vec3 rotation = transform->GetRotation();
//
//			//// Forward vector (local -Z)
//			//Math::vec3 forward = rotation * Math::vec3(0.0f, 0.0f, -1.0f);
//			//// Up vector (local +Y)
//			//Math::vec3 up = rotation * Math::vec3(0.0f, 1.0f, 0.0f);
//
//			//return Math::LookAtL(position, position + forward, up);
//		}
//
//		return Math::Identity<float>();
//	}
//
//	Math::mat4<float> GetProjectionMatrix() const
//	{
//		if (_isProjectionDirty)
//		{
//			_proj = Math::Projection(Math::Radians(_fov), _aspect, _nearPlane, _farPlane);
//			_isProjectionDirty = false;
//		}
//
//		return _proj;
//	}
//};