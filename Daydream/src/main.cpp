#include <Imgn.hpp>
#include <Imgn/main.hpp>

class TestLayer : public Imgn::Layer
{

public:
	TestLayer() : Layer("Test")
	{
	}

	TestLayer(Imgn::ImgnWindow* pWindow, Imgn::ImgnRenderer* pRenderer) : Layer("Test")
	{
	}

	void Dream(Imgn::Time pTime) override
	{
	}

	void OnEvent(Event& pEvent) override
	{
		EventDispatcher dispatcher(pEvent);
		dispatcher.Dispatch<KeyPressedEvent>(IMGN_BIND_EVENT_FN(TestLayer::OnKeyPressedEvent));
	}

	bool OnKeyPressedEvent(KeyPressedEvent& pEvent)
	{
		return false;
	}

	//Imgn::PerspectiveCamera& GetCamera() { return _camera; }
	/* Class Functions */

};

class Daydream : public Imgn::ImgnApp
{
public:
	Daydream()
	{
		AddLayer(Unique<TestLayer>(&GetWindow(), &Renderer())); //treating as my rendering layer for now
	}

	~Daydream()
	{

	}
};

Imgn::ImgnApp* Imgn::CreateApplication()
{
	return new Daydream();
}