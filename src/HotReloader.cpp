#include "D:/GitHub/Imagination-Engine/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
#include "HotReloader.h"
using namespace std;

void HotReloader::WatchFile(const filesystem::path& path, CallBack callBack)
{
	lock_guard<mutex> lock(_mutex);
	_watched[path] = { filesystem::last_write_time(path), move(callBack) };
}

void HotReloader::Poll()
{
	vector<pair<filesystem::path, CallBack>> changed;

	{
		lock_guard<mutex> lock(_mutex);

		for (auto& watch : _watched)
		{
			auto now = filesystem::last_write_time(watch.first);

			if (now != watch.second._fileTimeType)
			{
				watch.second._fileTimeType = now;
				changed.emplace_back(watch.first, watch.second.callBack);
			}
		}
	}

	for (auto& change : changed)
	{
		_threadPool.Enqueue([change] {change.second(change.first); });
	}
}