#pragma once
#include "ComponentTypes.h"
#include "Events/AppEvent.h"
#include "Events/KeyEvent.h"

class Camera : public Component, public EventListener
{
	//CameraComponent* _camera;

	float _moveSpeed = 5.f, _rotateSpeed = 0.1f;

	bool _moveForward = false, _moveBackward = false, _moveLeft = false, _moveRight = false;

	//EventBus& _eventBus = Device::Inst().GetEventBus();

public:
	void Init() override
	{
		//_camera = GetOwner()->GetComponent<CameraComponent>();

		// Register as event listener
		//_eventBus.AddListener(this);
	}

	void Update(float pDeltaTime) override
	{
		//if (!_camera) return;

		//// Handle movement
		//Math::vec3<float> movement = {};

		//if (_moveForward) movement.z -= 1.0f;
		//if (_moveBackward) movement.z += 1.0f;
		//if (_moveLeft) movement.x -= 1.0f;
		//if (_moveRight) movement.x += 1.0f;

		//if (movement.Length() > 0.0f)
		//{
		//	//movement = movement.Normalize() * _moveSpeed * pDeltaTime;

		//	auto transform = GetOwner()->GetComponent<TransformComponent>();
		//	if (transform)
		//	{
		//		Math::vec3 position = transform->GetPosition();
		//		position += movement;
		//		transform->SetPosition(position);
		//	}
		//}
	}

	void OnEvent(Event& event) override
	{
		EventDispatcher dispatcher(event);

		// Handle key press events
		dispatcher.Dispatch<KeyPressedEvent>([this](const KeyPressedEvent& e)
			{
				switch (e.GetKeyCode())
				{
				//case KEY_W: _moveForward = true; break;
				//case KEY_S: _moveBackward = true; break;
				//case KEY_A: _moveLeft = true; break;
				//case KEY_D: _moveRight = true; break;
				}

				return false;
			});

		// Handle key release events
		dispatcher.Dispatch<KeyReleasedEvent>([this](const KeyReleasedEvent& e)
			{
				switch (e.GetKeyCode())
				{
				//case KEY_W: _moveForward = false; break;
				//case KEY_S: _moveBackward = false; break;
				//case KEY_A: _moveLeft = false; break;
				//case KEY_D: _moveRight = false; break;
				}

				return false;
			});

		// Handle window resize events
		dispatcher.Dispatch<WindowResizedEvent>([this](const WindowResizedEvent& e)
			{
				//if (_camera)
				//{
				//	float aspectRatio = static_cast<float>(e.GetWidth()) / static_cast<float>(e.GetHeight());
				//	// _camera->SetAspectRatio(aspectRatio);
				//}

				return false;
			});
	}

	~Camera() override
	{
		// Unregister as event listener
		//_eventBus.RemoveListener(this);
	}
};

