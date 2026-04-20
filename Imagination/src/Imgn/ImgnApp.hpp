#pragma once
#include "ImgnCore.hpp"
#include "ImgnLog.h"

class IMGN_API ImgnApp
{
public:
	ImgnApp()
	{
		//std::format("{}:{}:{}", std)
		IMGN_CORE_TRACE("...")
			IMGN_INFO("...")
			IMGN_WARN("...")
			IMGN_ERROR("...")
			IMGN_FATAL("...")
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