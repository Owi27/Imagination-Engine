struct FrameGraphNode
{
	std::string name; //node name
	std::vector<std::string> inputResources;         
	std::vector<std::string> outputResources;        
	std::function<void(VkCommandBuffer)> Execute;    
};

struct FrameGraphResource
{
	std::string name;       
	bool isExternal;        
	VkImage image;          
	VkImageView imageView;
	Buffer* buffer;
	VkFormat format;        
	VkExtent2D extent;      
};

class FrameGraph
{
	static inline FrameGraph* _frameGraph = nullptr;
	std::vector<FrameGraphNode> _nodes;
	std::unordered_map<std::string, FrameGraphResource> _resources;

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
		for (const auto& node : _nodes) {
			// Ensure input resources are prepared here if needed
			node.Execute(commandBuffer);
		}
	}
};