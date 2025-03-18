#include "pch.h"
#include "FrameGraph.h"

RenderPass& FrameGraph::AddPass(const std::string& name, FrameGraphQueueBit queue)
{
	if (_passes.find(name) != _passes.end()) return *_passes[name];
	else
	{
		auto& pass = _passes[name] = std::make_unique<RenderPass>(*this, queue);
		pass->SetName(name);
		return *pass;
	}
}

Texture& FrameGraph::GetTextureResource(const std::string& name)
{
	if (_textureResources.find(name) != _textureResources.end()) return *_textureResources[name];
	else
	{
		_textureResources[name] = std::make_unique<Texture>();
		_textureResources[name]->SetName(name);
		return *_textureResources[name];
	}
}

Buffer& FrameGraph::GetBufferResource(const std::string& name)
{
	if (_bufferResources.find(name) != _bufferResources.end()) return *_bufferResources[name];
	else
	{
		_bufferResources[name] = std::make_unique<Buffer>();
		_bufferResources[name]->SetName(name);
		return *_bufferResources[name];
	}
}
