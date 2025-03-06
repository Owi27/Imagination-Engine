#include "pch.h"
#include "FrameGraph.h"

void FrameGraph::Bake()
{
	for (auto& pass : _passes)
	{
		pass.get()->Setup();
	}
}