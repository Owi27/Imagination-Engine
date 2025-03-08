//#include "FrameGraph.h"
//
//
//
//struct FrameGraphResource
//{
//	std::string name = "";
//	std::string parent = "";
//	//bool isExternal = false; // don't need?
//	bool prepared = false;
//
//	virtual ~FrameGraphResource() = default; // Virtual destructor
//};
//
//struct FrameGraphImageResource : FrameGraphResource
//{
//	Texture image;
//	VkFormat format;
//	VkExtent3D extent;
//};
//
//template <typename T>
//struct FrameGraphBufferResource : FrameGraphResource
//{
//	std::vector<T> data;
//	std::vector<Buffer> buffers;
//};
//
//using BufferDataVariants = std::variant<
//	FrameGraphBufferResource<UniformBufferOffscreen>,
//	FrameGraphBufferResource<UniformBufferFinal>,
//	FrameGraphBufferResource<Vertex>,
//	FrameGraphBufferResource<unsigned int>
//>;
//
//struct FrameGraphNode
//{
//	std::string name; //node name
//	bool shouldExecute = false;
//	bool isSetupComplete = false;
//	FrameBufferT frameBuffer;
//	std::vector<std::string> inputResources;
//	std::vector<std::string> outputResources;
//	std::function<void(FrameGraphNode&)> Setup;
//	std::function<void(VkCommandBuffer&, FrameGraphNode&)> Execute;
//};
//
//class FrameGraph
//{
//	static inline FrameGraph* _frameGraph = nullptr;
//	std::vector<std::unique_ptr<RenderPass>> _passes;
//
//	std::map<std::string, std::unique_ptr<RenderPass>> _test;
//	std::vector<std::string> _passStack;
//
//	std::vector<std::unique_ptr<FrameGraphResource>> _resources;
//	std::unordered_map<std::string, unsigned int> _passToIndex;
//	std::vector<FrameGraphNode> _nodes;
//	std::vector<VkSemaphore> _semaphores;
//	std::unordered_map<std::string, BufferDataVariants> _bufferResources;
//	std::unordered_map<std::string, FrameGraphImageResource> _imageResources;
//
//	FrameGraph() {};
//	~FrameGraph() {};
//
//public:
//
//	static FrameGraph* GetInstance()
//	{
//		if (!_frameGraph) _frameGraph = new FrameGraph();
//
//		return _frameGraph;
//	}
//
//	int GetNodeCount() const
//	{
//		return _nodes.size();
//	}
//
//	void AddNode(const FrameGraphNode& frameGraphNode)
//	{
//		_nodes.push_back(frameGraphNode);
//	}
//
//	template <typename T>
//	void AddBufferResource(const std::string& name, FrameGraphBufferResource<T>& resource)
//	{
//		_bufferResources[name] = resource;
//	}
//
//	void AddImageResource(const std::string& name, FrameGraphImageResource& resource)
//	{
//		_imageResources[name] = resource;
//	}
//
//	template <typename T>
//	FrameGraphBufferResource<T>& GetBufferResource(const std::string& name)
//	{
//		return std::get<FrameGraphBufferResource<T>>(_bufferResources.at(name));
//	}
//
//	FrameGraphImageResource& GetImageResource(const std::string& name)
//	{
//		return _imageResources.at(name);
//	}
//
//	void Execute(VkCommandBuffer& commandBuffer)
//	{
//		for (auto& node : _nodes) {
//			// Ensure input resources are prepared here if needed
//			if (node.shouldExecute)
//			{
//				if (!node.isSetupComplete) node.Setup(node);
//
//				node.Execute(commandBuffer, node);
//			}
//		}
//	}
//
//	RenderPass& AddPass(const std::string& name, FrameGraphQueueBit framegraphQueueBit)
//	{
//		for (auto& p : _test)
//		{
//			auto& pp = *p.second;
//
//		}
//
//		auto itr = _passToIndex.find(name);
//		if (itr != _passToIndex.end())
//		{
//			return *_passes[itr->second];
//		}
//		else
//		{
//			unsigned index = _passes.size();
//			_passes.emplace_back(new RenderPass());
//			//set pass name
//			_passToIndex[name] = index;
//			return *_passes.back();
//		}
//
//		if (_test.find(name) != _test.end())
//		{
//			return *_test[name];
//		}
//		else
//		{
//			_test.emplace(name, new RenderPass);
//			return *_test[name];
//		}
//	}
//
//	void Reset()
//	{
//		_passes.clear();
//		_resources.clear();
//	}
//};