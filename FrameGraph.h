#pragma once
enum FrameGraphQueueBit
{
	FRAMEGRAPH_GRAPHICS_BIT = 1 << 0,
	FRAMEGRAPH_COMPUTE_BIT = 1 << 1
};

class FrameGraph
{

	std::vector<std::unique_ptr<RenderPass>> _passes;

public:
	FrameGraph()
	{

	}

	~FrameGraph()
	{

	}

	RenderPass& AddPass(const std::string& name, FrameGraphQueueBit queue);
};

