

struct FrameGraphResource
{
	std::string name = "";
	//bool isExternal = false; // don't need?
	bool prepared = false;

	virtual ~FrameGraphResource() = default; // Virtual destructor
};

struct FrameGraphImageResource : FrameGraphResource
{
	Image image;
	VkFormat format;
	VkExtent3D extent;
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
	std::function<void()> Setup;
	std::function<void(VkCommandBuffer)> Execute;
};

class FrameGraph
{
	VkDevice _device;
	VkPhysicalDevice _physicalDevice;
	static inline FrameGraph* _frameGraph = nullptr;
	std::vector<FrameGraphNode> _nodes;
	std::unordered_map<std::string, FrameGraphResource*> _resources;

	FrameGraph() {};
	~FrameGraph() {};
	// Deleting the copy constructor to prevent copies
	FrameGraph(const FrameGraph& frameGraph) = delete;

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

	void AddResource(const std::string& name, FrameGraphResource* resource)
	{
		_resources[name] = resource;
		_resources[name]->prepared = true;
	}

	FrameGraphResource* GetResource(const std::string& name)
	{
		return _resources.at(name);
	}

	void Execute(VkCommandBuffer commandBuffer)
	{
		for (auto& node : _nodes) {
			// Ensure input resources are prepared here if needed
			if (node.shouldExecute)
			{
				//CreateFrameBuffer(node);

				node.Execute(commandBuffer);
			}
		}
	}

	void SetDevice(VkDevice& device) { _device = device; }
	void SetPhysicalDevice(VkPhysicalDevice physicalDevice) { _physicalDevice = physicalDevice; }
};