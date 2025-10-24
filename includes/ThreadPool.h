#pragma once
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

class ThreadPool
{
	std::vector<std::thread> _threads;
	std::queue<std::function<void()>> _tasks;
	std::mutex _mutex; //to sync access to shared data
	std::condition_variable _conditionVariable; //signal change in tasks
	bool _stop = false; //flag to stop threadpool

public:
	ThreadPool(int threadCount = std::thread::hardware_concurrency())
	{
		//create worker threads
		for (size_t i = 0; i < threadCount; i++)
		{
			_threads.emplace_back([this]
				{
					while (true)
					{
						std::function<void()> task;

						//unlock queue before executing task so other threads can perform enqueue tasks
						{
							//lock queue so data can be shared safely
							std::unique_lock<std::mutex> lock(_mutex);

							//wait until theres a task or pool is stopped
							_conditionVariable.wait(lock, [this] { return !_tasks.empty() || _stop; });

							//exit thread if pool is stopped and no tasks
							if (_stop && _tasks.empty()) return;

							//get next task from queue
							task = move(_tasks.front());
							_tasks.pop();
						}

						task();
					}
				});
		}
	}

	~ThreadPool()
	{
		{
			//lock queue to update the stop flag
			std::unique_lock<std::mutex> lock(_mutex);
			_stop = true;
		}

		//notify threads
		_conditionVariable.notify_all();

		//join all threads
		for (std::thread& t : _threads) t.join();
	}

	void Enqueue(std::function<void()> task);
};

