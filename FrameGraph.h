

struct FrameGraphResource
{
	std::string name = "";
	bool isExternal = false; // don't need?
	bool prepared = false;

	virtual ~FrameGraphResource() = default; // Virtual destructor
};

struct FrameGraphImageResource : FrameGraphResource
{
	Image image;
	VkFormat format;
	VkExtent2D extent;
};

struct FrameGraphBufferResource : FrameGraphResource
{
	std::vector<Buffer> buffers;
};

struct FrameGraphNode
{
	std::string name; //node name
	bool shouldExecute = false;
	FrameBufferT frameBuffer;
	std::vector<std::string> inputResources;
	std::vector<std::string> outputResources;
	std::function<void(VkCommandBuffer)> Execute;
};

class FrameGraph
{
	VkDevice _device;
	VkPhysicalDevice _physicalDevice;
	static inline FrameGraph* _frameGraph = nullptr;
	std::vector<FrameGraphNode> _nodes;
	std::unordered_map<std::string, FrameGraphResource> _resources;

	FrameGraph() {};
	~FrameGraph() {};
	// Deleting the copy constructor to prevent copies
	FrameGraph(const FrameGraph& frameGraph) = delete;

	void CreateFrameBuffer(FrameGraphNode& node)
	{
		std::vector<VkAttachmentDescription> attachmentDescriptions;
		std::vector<VkAttachmentReference> colorAttachmentReferences;
		VkAttachmentReference depthAttachmentReference;
		unsigned int width = 1, height = 1;

		for (const auto& resourceName : node.outputResources)
		{
			const auto& resource = _resources.at(resourceName);

			if (const FrameGraphImageResource* imageResource = dynamic_cast<const FrameGraphImageResource*>(&resource))
			{
				width = imageResource->extent.width;
				height = imageResource->extent.height;

				VkAttachmentDescription attachmentDescription;
				attachmentDescription.format = imageResource->format;
				attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				if (resourceName == "Depth Buffer")
				{
					attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					depthAttachmentReference.attachment = static_cast<uint32_t>(attachmentDescriptions.size());
					depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					//hasDepthAttachment = true;
				}
				else
				{
					VkAttachmentReference colorReference;
					colorReference.attachment = static_cast<uint32_t>(attachmentDescriptions.size());
					colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					colorAttachmentReferences.push_back(colorReference);
				}

				attachmentDescriptions.push_back(attachmentDescription);
			}
		}

		// Define the subpass
		VkSubpassDescription subpassDescription;
		subpassDescription.flags = 0;
		subpassDescription.pipelineBindPoint = node.frameBuffer.bindPoint;
		subpassDescription.pColorAttachments = colorAttachmentReferences.data();
		subpassDescription.colorAttachmentCount = colorAttachmentReferences.size();
		subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pResolveAttachments = nullptr;

		// Use subpass dependencies for attachment layout transitions
		std::array<VkSubpassDependency, 2> subpassDependencies;

		subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[0].dstSubpass = 0;
		subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		subpassDependencies[1].srcSubpass = 0;
		subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
		renderPassCreateInfo.attachmentCount = attachmentDescriptions.size();
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = subpassDependencies.size();
		renderPassCreateInfo.pDependencies = subpassDependencies.data();

		vkCreateRenderPass(_device, &renderPassCreateInfo, nullptr, &node.frameBuffer.renderPass);

		std::vector<VkImageView> attachments;
		for (const auto& resourceName : node.outputResources)
		{
			const auto& resource = _resources.at(resourceName);

			if (const FrameGraphImageResource* imageResource = dynamic_cast<const FrameGraphImageResource*>(&resource))
			{
				attachments.push_back(imageResource->image.imageView);
			}
		}

		VkFramebufferCreateInfo framebufferCreateInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = node.frameBuffer.renderPass;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = width;
		framebufferCreateInfo.height = height;
		framebufferCreateInfo.layers = 1; // Single-layer framebuffer

		vkCreateFramebuffer(_device, &framebufferCreateInfo, nullptr, &node.frameBuffer.frameBuffer);
	}

public:

	static FrameGraph* GetInstance()
	{
		if (!_frameGraph) _frameGraph = new FrameGraph();

		return _frameGraph;
	}

	void AddNode(const FrameGraphNode& frameGraphNode)
	{
		_nodes.push_back(frameGraphNode);
	}

	void AddResource(const std::string& name, const FrameGraphResource& resource)
	{
		_resources[name] = resource;
	}

	const FrameGraphResource& GetResource(const std::string& name) const
	{
		return _resources.at(name);
	}

	void Execute(VkCommandBuffer commandBuffer)
	{
		for (auto& node : _nodes) {
			// Ensure input resources are prepared here if needed
			if (node.shouldExecute)
			{
				CreateFrameBuffer(node);

				node.Execute(commandBuffer);
			}
		}
	}

	void SetDevice(VkDevice& device) { _device = device; }
	void SetPhysicalDevice(VkPhysicalDevice physicalDevice) { _physicalDevice = physicalDevice; }
};