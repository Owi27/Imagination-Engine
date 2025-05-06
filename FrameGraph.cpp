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

void FrameGraph::Execute()
{
	for (auto& pass : _passes)
	{
		pass.second->BuildCommandBuffer();
		//_passOrder.push_back(pass.second->GetName());
	}

	for (size_t i = 0; i < _passOrder.size(); i++)
	{
		_passes[_passOrder[i]]->Update();

		if (i == 0)
		{
			_vk.SubmitQueue(_vk.GetSemaphore(), _passes[_passOrder[i]]->GetSemaphore(), _passes[_passOrder[i]]->GetCommandBuffer());
		}
		else if (i == _passOrder.size() - 1)
		{
			_vk.SubmitQueue(_passes[_passOrder[i - 1]]->GetSemaphore(), _passes[_passOrder[i]]->GetSemaphore(), _passes[_passOrder[i]]->GetCommandBuffer(), _vk.GetCurrentFence());
			_vk.PresentInfo(_passes[_passOrder[i]]->GetSemaphore());
		}
		else
		{
			_vk.SubmitQueue(_passes[_passOrder[i - 1]]->GetSemaphore(), _passes[_passOrder[i]]->GetSemaphore(), _passes[_passOrder[i]]->GetCommandBuffer());
		}
	}

	//_guiContext->BuildCommandBuffer();

	//ui
	/*_vk.SubmitQueue(_passes.end()->second->GetSemaphore(), _guiContext->GetSemaphore(), _guiContext->GetCommandBuffer(), _vk.GetCurrentFence());
	_vk.PresentInfo(_guiContext->GetSemaphore());*/

	//	RunCBs();
}

void FrameGraph::BuildCommandBuffers()
{
	for (auto& pass : _passes)
	{
		pass.second->Setup();
	}

	for (auto& pass : _passes)
	{
		pass.second->BuildCommandBuffer();
		//_passOrder.push_back(pass.second->GetName());
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
