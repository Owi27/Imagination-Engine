#include <Imgn.hpp>

struct TestComp : public ImgnComponent
{
	TestComp() : ImgnComponent("Tester")
	{
		
	}
	void Dream(float pDeltaTime) override
	{
		IMGN_INFO("{} Test Comp Update", GetName());
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