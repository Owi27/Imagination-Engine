#include "pch.h"
#include "FrameGraph.h"

void FrameGraph::Bake()
{
	for (auto& pass : _passes)
	{
		pass.get()->Setup();
	}
}

RenderPass& FrameGraph::AddPass(const std::string& name, FrameGraphQueueBit queue)
{
	RenderPass p;
	return p;
}
