


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

template <typename T>
struct FrameGraphBufferResource : FrameGraphResource
{
	std::vector<T> data;
	std::vector<Buffer> buffers;
};

using BufferDataVariants = std::variant<
	FrameGraphBufferResource<UniformBufferOffscreen>,
	FrameGraphBufferResource<UniformBufferFinal>,
	FrameGraphBufferResource<Vertex>,
	FrameGraphBufferResource<unsigned int>
>;

struct FrameGraphNode
{
	std::string name; //node name
	bool shouldExecute = false;
	bool isSetupComplete = false;
	FrameBufferT frameBuffer;
	std::vector<std::string> inputResources;
	std::vector<std::string> outputResources;
	std::function<void(FrameGraphNode&)> Setup;
	std::function<void(VkCommandBuffer&, FrameGraphNode&)> Execute;
};

class FrameGraph
{
	static inline FrameGraph* _frameGraph = nullptr;
	std::vector<FrameGraphNode> _nodes;
	std::vector<VkSemaphore> _semaphores;
	std::unordered_map<std::string, BufferDataVariants> _bufferResources;
	std::unordered_map<std::string, FrameGraphImageResource> _imageResources;

	FrameGraph() {};
	~FrameGraph() {};

public:

	static FrameGraph* GetInstance()
	{
		if (!_frameGraph) _frameGraph = new FrameGraph();

		return _frameGraph;
	}

	int GetNodeCount() const
	{
		return _nodes.size();
	}

	void AddNode(const FrameGraphNode& frameGraphNode)
	{
		_nodes.push_back(frameGraphNode);
	}

	template <typename T>
	void AddBufferResource(const std::string& name, FrameGraphBufferResource<T>& resource)
	{
		_bufferResources[name] = resource;
	}

	void AddImageResource(const std::string& name, FrameGraphImageResource& resource)
	{
		_imageResources[name] = resource;
	}

	template <typename T>
	FrameGraphBufferResource<T>& GetBufferResource(const std::string& name)
	{
		return std::get<FrameGraphBufferResource<T>>(_bufferResources.at(name));
	}

	FrameGraphImageResource& GetImageResource(const std::string& name)
	{
		return _imageResources.at(name);
	}

	void Execute(VkCommandBuffer& commandBuffer)
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