#include <Imgn.hpp>

struct TestComp : public ImgnComponent
{
	TestComp() : ImgnComponent("Tester")
	{


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

		std::vector vertices =
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

		uint32_t vertexBuffer = Renderer().CreateVertexBuffer(vertices);



	}

	~Daydream()
	{

	}
};

ImgnApp* IMGN::CreateApplication()
{
	return new Daydream();
}