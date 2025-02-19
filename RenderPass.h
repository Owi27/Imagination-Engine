#pragma once
class RenderPass
{

public:
	RenderPass() = default;
	~RenderPass() = default;

	virtual void Setup() = 0;
	virtual void Execute() = 0;
};

