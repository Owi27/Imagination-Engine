

struct FrameGraphResource
{
	std::string name = "";
	std::string parent = "";
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
	std::vector<void*> data;
	std::vector<Buffer> buffers;
};

struct FrameGraphNode
{
	std::string name; //node name
	bool shouldExecute = false;
	bool isSetupComplete = false;
	FrameBufferT frameBuffer;
	std::vector<std::string> inputResources;
	std::vector<std::string> outputResources;
	std::function<void(FrameGraphNode&)> Setup;
	std::function<void(VkCommandBuffer, FrameGraphNode&)> Execute;
};

class FrameGraph
{
	static inline FrameGraph* _frameGraph = nullptr;
	std::vector<FrameGraphNode> _nodes;
	std::unordered_map<std::string, FrameGraphResource*> _resources; //split

	FrameGraph() {};
	~FrameGraph() {};

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
				if (!node.isSetupComplete) node.Setup(node);

				node.Execute(commandBuffer, node);
			}
		}
	}
};