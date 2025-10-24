#pragma once
#include <filesystem>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <functional>
#include <optional>
#include "ThreadPool.h"

class HotReloader
{
	using CallBack = std::function<void(const std::filesystem::path&)>;
	
	struct Entry
	{
		std::filesystem::file_time_type _fileTimeType;
		CallBack callBack;
	};

	ThreadPool& _threadPool;
	std::mutex _mutex;
	std::unordered_map<std::filesystem::path, Entry> _watched;

public:
	HotReloader(ThreadPool& threadPool) : _threadPool(threadPool)
	{

	}

	void WatchFile(const std::filesystem::path& path, CallBack callBack);
	void Poll();
};

