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
	VulkanContext& _vk = *VulkanContext::GetInst();
	std::unordered_map<std::string, std::unique_ptr<RenderPass>> _passes;
	std::unordered_map<std::string, std::unique_ptr<Texture>> _textureResources;
	std::unordered_map<std::string, std::unique_ptr<Buffer>> _bufferResources;

	std::vector<std::string> _passOrder = { "skybox", "offscreen", "lighting", "swapchain" };

public:
	FrameGraph()
	{

	}

	~FrameGraph()
	{

	}

	FrameGraphBlackboard _blackboard;

	RenderPass& AddPass(const std::string& name, FrameGraphQueueBit queue);

	VkSemaphore& GetSemaphore() { return _passes["offscreen"]->GetSemaphore(); }
	VkCommandBuffer& GetCB() { return _passes["offscreen"]->GetCommandBuffer(); }
	VkSemaphore& GetSemaphore2() { return _passes["lighting"]->GetSemaphore(); }
	VkCommandBuffer& GetCB2() { return _passes["lighting"]->GetCommandBuffer(); }

	void Execute();
	void BuildCommandBuffers();

	Texture& GetTextureResource(const std::string& name);
	Buffer& GetBufferResource(const std::string& name);
};