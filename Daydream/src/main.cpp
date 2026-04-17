#include <Imgn.hpp>

class Daydream : public ImgnApp
{
public:
	Daydream()
	{

	}

	~Daydream()
	{

	}
};

ImgnApp* IMGN::CreateApplication()
{
	return new Daydream();
}