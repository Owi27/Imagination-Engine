#pragma once
class RenderPass;
class RenderTarget;

//enum class RenderPassIdx : uint32_t
//{
//    GBuffer = 0,
//    Lighting = 1,
//    Count
//};

class RenderPassManager
{
	std::unordered_map<std::string, std::unique_ptr<RenderPass>> _renderPasses;
	std::vector<RenderPass*> _sortedPasses;
	bool _isDirty = true;

    void SortPasses();
    void TopologicalSort(const std::string& pName, const std::unordered_map<std::string, RenderPass*>& pPassMap, std::unordered_set<std::string>& pVisited, std::unordered_set<std::string>& pVisiting);

public:
    template<typename T, typename... Args>
    T* AddRenderPass(const std::string& pName, Args&&... pArgs)
    {
        static_assert(std::is_base_of<RenderPass, T>::value, "T must derive from RenderPass");

        auto it = _renderPasses.find(pName);
        if (it != _renderPasses.end())
        {
            return dynamic_cast<T*>(it->second.get());
        }

        auto pass = std::make_unique<T>(std::forward<Args>(pArgs)...);
        T* passPtr = pass.get();
        _renderPasses[pName] = std::move(pass);
        _isDirty = true;

        return passPtr;
    }

    RenderPass* GetRenderPass(const std::string& pName);
    void RemoveRenderPass(const std::string& pName);
    void Execute(vk::raii::CommandBuffer& pCommandBuffer);
};

class RenderPass
{
    std::string _name;
    std::vector<std::string> _dependencies;
    RenderTarget* _target = nullptr;
    bool _isEnabled = true;

    // With dynamic rendering, BeginPass typically calls vkCmdBeginRendering
    // instead of vkCmdBeginRenderPass
    virtual void BeginPass(vk::raii::CommandBuffer& pCommandBuffer) = 0;
    virtual void Render(vk::raii::CommandBuffer& pCommandBuffer) = 0;
    // With dynamic rendering, EndPass typically calls vkCmdEndRendering
    // instead of vkCmdEndRenderPass
    virtual void EndPass(vk::raii::CommandBuffer& pCommandBuffer) = 0;

public:
    explicit RenderPass(const std::string& pPassName)
    {
        _name = pPassName;
    }

    virtual ~RenderPass() = default;

    const std::string& GetName() const { return _name; }

    void AddDependency(const std::string& pDependency)
    {
        _dependencies.push_back(pDependency);
    }

    const std::vector<std::string>& GetDependencies() const
    {
        return _dependencies;
    }

    void SetRenderTarget(RenderTarget* pRenderTarget)
    {
        _target = pRenderTarget;
    }

    RenderTarget* GetRenderTarget() const
    {
        return _target;
    }

    void SetEnabled(bool pIsEnabled)
    {
        _isEnabled = pIsEnabled;
    }

    bool IsEnabled() const
    {
        return _isEnabled;
    }

    virtual void Execute(vk::raii::CommandBuffer& pCommandBuffer);
};

class RenderTarget
{
    vk::raii::Image _colorImage = nullptr;
    vk::raii::DeviceMemory _colorMemory = nullptr;
    vk::raii::ImageView _colorImageView = nullptr;

    vk::raii::Image _depthImage = nullptr;
    vk::raii::DeviceMemory _depthMemory = nullptr;
    vk::raii::ImageView _depthImageView = nullptr;

    uint32_t _width, _height;

    void CreateColorResources();
    void CreateDepthResources();

public:
    RenderTarget(uint32_t pWidth, uint32_t pHeight)
    {
        _width = pWidth;
        _height = pHeight;

        // Create color and depth images
        CreateColorResources();
        CreateDepthResources();

        // Note: With dynamic rendering, we don't need to create VkRenderPass
        // or VkFramebuffer objects. Instead, we just create the images and
        // image views that will be used directly with vkCmdBeginRendering.
    }

    ~RenderTarget() /*Destructor*/
    {
    }

    vk::ImageView GetColorImageView() const { return *_colorImageView; }
    vk::ImageView GetDepthImageView() const { return *_depthImageView; }

    uint32_t GetWidth() const { return _width; }
    uint32_t GetHeight() const { return _height; }
};

//class GeometryPass : public RenderPass
//{
//    RenderTarget* gBuffer;
//
//protected:
//    void BeginPass(vk::raii::CommandBuffer& pCommandBuffer) override;
//    void Render(vk::raii::CommandBuffer& pCommandBuffer) override;
//    void EndPass(vk::raii::CommandBuffer& pCommandBuffer) override;
//
//public:
//    GeometryPass(const std::string& pName) : RenderPass(pName) /*Constructor*/
//    {
//        gBuffer = new RenderTarget(800, 600);
//        SetRenderTarget(gBuffer);
//    }
//
//    ~GeometryPass() override /*Destructor*/
//    {
//        delete gBuffer;
//    }
//
//};