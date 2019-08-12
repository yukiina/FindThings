#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threads) :_stop(false) {
	for (size_t i = 0; i < threads; i++) {
		_workes.emplace_back([this]() {
			for (;;) {
				Task task;
				{
					std::unique_lock<mutex> ulc(_mutex);
					_cond.wait(ulc, [this]() {return _stop || !_tasks.empty(); });

					if (_stop && _tasks.empty())
						return;

					task = std::move(_tasks.front());
					_tasks.pop();
				}
				task();
			}

			});
	}
}

ThreadPool::~ThreadPool() {
	{
		std::unique_lock<mutex> ulc(_mutex);
		_stop = true;
	}
	_cond.notify_all();

	for (auto& thread : _workes) {
		thread.join();
	}
}
