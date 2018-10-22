#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <list>
#include <functional>
#include <cstdio>
#include <vector>

class ThreadPool
{
public:
	using Task = std::function<void(void)>;
	explicit ThreadPool(int num);
	explicit ThreadPool();
	void Init(int num);
	~ThreadPool();
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool &rhs) = delete;
	void append(const Task &task);
	void append(Task &&task);
	void start();
	void stop();
	bool finishTask();

private:
	bool isrunning;
	int threadNum;
	bool noTask;
	void work(int id);
	std::mutex m;
	std::mutex newmutex;
	std::condition_variable cond;
public:std::list<Task> tasks;
	std::vector<std::shared_ptr<std::thread>> threads;
};
