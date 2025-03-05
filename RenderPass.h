#pragma once
class Texture;

class RenderPass
{
	FrameGraph _graph;
	std::string _name;
	FrameGraphQueueBit _queue;
	std::vector<std::shared_ptr<Texture>> _colorInputs;
	std::vector<std::shared_ptr<Texture>> _colorOutputs;
	std::shared_ptr<Texture> _depthStencilInput = nullptr;
	std::shared_ptr<Texture> _depthStencilOutput = nullptr;

public:
	RenderPass(FrameGraph& graph, FrameGraphQueueBit queue)
	{
		_graph = graph;
		_queue = queue;
	}

	~RenderPass()
	{

	}

	Texture& AddTextureInput(std::string& name);
	// should have color attachments
	virtual void Setup();
	virtual void Execute();
};

