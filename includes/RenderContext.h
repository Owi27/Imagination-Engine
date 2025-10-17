#pragma once
#include <windows.h>

class RenderContext
{
	
public:
	RenderContext()
	{

	}

	~RenderContext()
	{

	}

	virtual bool Init();
	virtual bool Prepare();
	virtual bool Render();
};

