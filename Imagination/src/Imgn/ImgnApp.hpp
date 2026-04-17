#pragma once
#include "ImgnCore.hpp"

class IMGN_API ImgnApp
{
public:
	ImgnApp()
	{

	}

	~ImgnApp()
	{

	}

	void Run();
};

namespace IMGN
{
	ImgnApp* CreateApplication();
}