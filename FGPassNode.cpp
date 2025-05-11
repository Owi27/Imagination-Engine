#include "FGPassNode.hpp"
#include <cassert>

namespace
{
	bool HasID(const std::vector<int>& v, int id)
	{
		return std::ranges::find(v, id) != v.end();
	}

	bool HasID(const std::vector<FGPassNode::AccessDeclaration>& v, int id)
	{
		const auto match = [id](const auto& e) { return e.id == id; };
		return std::ranges::find_if(v, match) != v.end();
	}

	bool Contains(const std::vector<FGPassNode::AccessDeclaration>& v, FGPassNode::AccessDeclaration d)
	{
		return std::ranges::find(v, d) != v.end();
	}
}

FGPassNode::FGPassNode(const std::string name, unsigned id, std::unique_ptr<FGPassBase>&& execute) : FGGraphNode{ name, id }, _executes(std::move(execute))
{
	_creates.reserve(10);
	_reads.reserve(10);
	_writes.reserve(10);
}

int FGPassNode::_Read(int id, unsigned flags)
{
	assert(!Creates(id) && !Writes(id));

	return Contains(_reads, { id, flags }) ? id : _reads.emplace_back(AccessDeclaration{ .id = id, .flags = flags }).id;
}

int FGPassNode::_Write(int id, unsigned flags)
{
	return Contains(_writes, { id, flags }) ? id : _writes.emplace_back(AccessDeclaration{ .id = id, .flags = flags }).id;
}

bool FGPassNode::Creates(int id) const
{
	return HasID(_creates, id);
}

bool FGPassNode::Reads(int id) const
{
	return HasID(_reads, id);
}

bool FGPassNode::Writes(int id) const
{
	return HasID(_writes, id);
}

bool FGPassNode::CanExecute() const
{
	return GetRefCount() > 0 || HasSideEffect();
}

FGGraphNode::FGGraphNode(const std::string name, unsigned id) : _id(id)
{
	_name = name;
}
