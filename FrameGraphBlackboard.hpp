#pragma once
#include <any>

class FrameGraphBlackboard
{
	std::unordered_map<std::string, std::any> _storage;
public:
	FrameGraphBlackboard() = default;
	FrameGraphBlackboard(const FrameGraphBlackboard&) = default;
	FrameGraphBlackboard(FrameGraphBlackboard&&) noexcept = default;
	~FrameGraphBlackboard() = default;
	FrameGraphBlackboard& operator=(const FrameGraphBlackboard&) = default;
	FrameGraphBlackboard& operator=(FrameGraphBlackboard&&) noexcept = default;

	template<typename T>
	T& Set(const std::string& name, const T& value)
	{
		_storage[name] = value;
		return std::any_cast<T&>(_storage[name]);
	}

	template<typename T>
	T& Get(const std::string& name)
	{
		return std::any_cast<T&>(_storage[name]);
	}

	void Remove(const std::string& name)
	{
		_storage.erase(name);
	}

	void Clear()
	{
		_storage.clear();
	}
};