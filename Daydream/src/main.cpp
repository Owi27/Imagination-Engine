#include <Imgn.hpp>

struct TestComp : public ImgnComponent
{
	TestComp() : ImgnComponent("Tester")
	{
		std::array vertices =
		{
			Vertex
			{
				.pos = { 0.f, .5f, 0.f}
			},
			Vertex
			{
				.pos = { .5f, -.5f, 0.f}
			},
			Vertex
			{
				.pos = { -.5f, -.5f, 0.f}
			}
		};

		unique<vk::raii::CommandBuffer> commandBuffer;


	}

	void Dream(float pDeltaTime) override
	{
		//IMGN_INFO("{} Test Comp Update", GetName());
	}

	/* Class Functions */
};
class Daydream : public ImgnApp
{
public:
	Daydream()
	{
		AddComponent<TestComp>();
	}

	~Daydream()
	{

	}
};

ImgnApp* IMGN::CreateApplication()
{
	return new Daydream();
}