#pragma once
#include "string"
#include <memory>
#include <vector>

class FGPassResources;

struct FGPassBase
{
	FGPassBase() = default;
	FGPassBase(const FGPassBase&) = delete;
	FGPassBase(FGPassBase&&) noexcept = delete;
	virtual ~FGPassBase() = default;

	FGPassBase& operator=(const FGPassBase&) = delete;
	FGPassBase& operator=(FGPassBase&&) noexcept = delete;

	virtual void operator()(FGPassResources&, void*) = 0;
};

template <typename Data, typename Execute>
struct FGPass :FGPassBase
{
	Execute executeFunction;
	Data data;

	explicit FGPass(Execute&& execute)
	{
		executeFunction = std::forward<Execute>(execute);
	}

	void operator()(FGPassResources& resources, void* context) override
	{
		executeFunction(data, resources, context);
	}
};

class FGGraphNode
{
	friend class FrameGraph;

	std::string _name;
	const unsigned _id;
	int _refCount = 0;

public:
	FGGraphNode() = delete;
	virtual ~FGGraphNode() = default;

	unsigned GetID() const { return _id; }
	std::string GetName() const { return _name; }
	int GetRefCount() const { return _refCount; }

protected:
	FGGraphNode(const std::string, unsigned);
};

class FGPassNode : FGGraphNode
{
	friend class FrameGraph;
	struct AccessDeclaration;

	std::unique_ptr<FGPassBase> _executes;
	std::vector<int> _creates;
	std::vector<AccessDeclaration> _reads;
	std::vector<AccessDeclaration> _writes;

	bool _hasSideEffect = false;

	FGPassNode(const std::string, unsigned, std::unique_ptr<FGPassBase>&&);
	
	int _Read(int, unsigned);
	int _Write(int, unsigned);

public:
	FGPassNode(const FGPassNode&) = delete;
	FGPassNode(FGPassNode&&) = default;
	FGPassNode& operator=(const FGPassNode&) = delete;
	FGPassNode& operator=(FGPassNode&&) = delete;

	struct AccessDeclaration
	{
		int id;
		unsigned flags;

		bool operator==(const AccessDeclaration&) const = default;
	};

	bool Creates(int) const;
	bool Reads(int) const;
	bool Writes(int) const;

	bool HasSideEffect() const { return _hasSideEffect; }
	bool CanExecute() const;

	struct Create{}; struct Read {}; struct Write {};
	std::vector<int> Each(const Create) const { return _creates; }
	std::vector<AccessDeclaration> Each(const Read) const { return _reads; }
	std::vector<AccessDeclaration> Each(const Write) const { return _writes; }
};

