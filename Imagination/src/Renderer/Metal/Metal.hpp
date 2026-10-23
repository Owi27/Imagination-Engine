#pragma once

#include "Renderer/IRenderBackend.hpp"

#import <Metal/Metal.h>

class Metal final : public IRenderBackend
{
	MTL::Device* _device = nullptr;
	MTL::CommandQueue* _commandQueue = nullptr;

	RendererCreateInfo _info;

public:
	~Metal() override
	{
		WakeUp();
	}

	bool Sleep(RendererCreateInfo pCreateInfo) override;
	void Dream() override;
	void WakeUp() override;

	id<MTLDevice> GetDevice() const { return _device; }
	id<MTLCommandQueue> GetCommandQueue() const { return _commandQueue; }
};