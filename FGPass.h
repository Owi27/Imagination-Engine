#pragma once

class FGPassBase
{
public:
    FGPassBase(const std::string& name, FGPassQueueType queueType)
        : _name(name), _queueType(queueType), _passIndexInExecutionOrder(UINT32_MAX), _hasExecutedThisFrame(false), _inDegree(0) {
    }
    virtual ~FGPassBase() = default;

    const std::string& GetName() const { return _name; }
    FGPassQueueType GetQueueType() const { return _queueType; }

    // The core execution logic for the pass.
    virtual void Execute(FrameGraph& frameGraph, VkCommandBuffer commandBuffer, FrameGraphBlackboard& blackboard) = 0;

    // --- Internal FrameGraph Management ---
    void SetPassIndexInExecutionOrder(uint32_t index) { _passIndexInExecutionOrder = index; }
    uint32_t GetPassIndexInExecutionOrder() const { return _passIndexInExecutionOrder; }

    // Stores all resource interactions (reads, writes, creates) declared by this pass.
    std::vector<FGResourceUsage> _resourceUsages;

    // For dependency tracking during compilation (topological sort).
    // `_dependencies`: Set of indices of passes that this pass directly depends on (its producers).
    // `_inDegree`: Number of incoming edges (producers) in the dependency graph. Used by Kahn's algorithm.
    // Note: In some topological sort implementations, _inDegree is directly manipulated.
    // Here, we'll calculate it based on _dependencies.
    // Or, more directly, _inDegree is the count of prerequisite passes.
    // `_dependentPasses`: Set of indices of passes that depend on this one (its consumers).
    std::set<uint32_t> _prerequisitePassIndices; // Passes that must execute before this one.
    uint32_t _inDegree; // Calculated during graph build: number of direct prerequisites.


    bool _hasExecutedThisFrame; // Flag to track execution status per framegraph execution.

protected:
    std::string _name;
    FGPassQueueType _queueType;
    uint32_t _passIndexInExecutionOrder; // Final index in the compiled execution order.
};

FGPassBase::FGPassBase()
{
}

FGPassBase::~FGPassBase()
{
}
class FGPass
{
public:
	FGPass()
	{

	}

	~FGPass()
	{

	}
};

