#include "D:/GitHub/Imagination-Engine/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
#include "ThreadPool.h"
using namespace std;

void ThreadPool::Enqueue(function<void()> task)
{
	{
		unique_lock<mutex> lock(_mutex);
		_tasks.emplace(move(task));
	}

	_conditionVariable.notify_one();
}
