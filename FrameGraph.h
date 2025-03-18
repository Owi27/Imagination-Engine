#pragma once
enum FrameGraphQueueBit
{
	FRAMEGRAPH_GRAPHICS_BIT = 1 << 0,
	FRAMEGRAPH_COMPUTE_BIT = 1 << 1
};


class Buffer;
class Texture;
class RenderPass;

class FrameGraph
{
	std::unordered_map<std::string, std::unique_ptr<RenderPass>> _passes;
	std::unordered_map<std::string, std::unique_ptr<Texture>> _textureResources;
	std::unordered_map<std::string, std::unique_ptr<Buffer>> _bufferResources;


public:
	FrameGraph()
	{
	}

	~FrameGraph()
	{

	}

	RenderPass& AddPass(const std::string& name, FrameGraphQueueBit queue);

	Texture& GetTextureResource(const std::string& name);
	Buffer& GetBufferResource(const std::string& name);
};