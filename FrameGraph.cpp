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
	auto pass = std::make_unique<RenderPass>(*this, queue);
	RenderPass& ref = *pass;
	_passes.push_back(std::move(pass));
	return ref;
}

Texture& FrameGraph::GetTextureResource(const std::string& name)
{
	if (_textureResources.find(name) != _textureResources.end()) return *_textureResources[name];
	else
	{
		_textureResources[name] = std::make_unique<Texture>(new Texture());
		_textureResources[name]->SetName(name);
		return *_textureResources[name];
	}
}

Buffer& FrameGraph::GetBufferResource(const std::string& name)
{
	if (_bufferResources.find(name) != _bufferResources.end()) return *_bufferResources[name];
	else
	{
		_bufferResources[name] = std::make_unique<Buffer>(new Buffer());
		_bufferResources[name]->SetName(name);
		return *_bufferResources[name];
	}
}
