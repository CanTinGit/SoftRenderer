#include "my_thread.h"
#include <ostream>
#include <iostream>
#include <synchapi.h>

ThreadPool::ThreadPool(int num): threadNum(num),isrunning(false),m(),noTask(false)
{
	
}
ThreadPool::ThreadPool() : threadNum(1), isrunning(false), m(), noTask(false)
{
}


ThreadPool::~ThreadPool()
{
	if (isrunning)
	{
		stop();
	}
}

void ThreadPool::Init(int num) 
{
	threadNum = num;
}

void ThreadPool::start()
{
	isrunning = true;
	threads.reserve(threadNum);
	for (int i = 0;i<threadNum;i++)
	{
		threads.push_back(std::make_shared<std::thread>(&ThreadPool::work, this, i));
	}
}

void ThreadPool::stop()
{
	{
		std::unique_lock<std::mutex> locker(m);
		isrunning = false;
		cond.notify_all();
	}
	for (int i = 0;i<threads.size();i++)
	{
		auto t = threads[i];
		if (t->joinable())
		{
			t->join();
		}
	}

}

bool ThreadPool::finishTask()
{
	//Sleep(1);
	std::cout << ""<<std::endl;
	return tasks.empty();
}


//添加任务，参数为左值
void ThreadPool::append(const Task& task)
{
	if (isrunning)
	{
		std::unique_lock<std::mutex> locker(m);
		tasks.push_back(task);
		cond.notify_one();   //唤醒一个进程来执行
	}
}

//添加任务，右值引用
void ThreadPool::append(Task&& task)
{
	if (isrunning)
	{
		std::unique_lock<std::mutex> locker(m);
		tasks.push_back(std::move(task));       //std::move语句可以将左值变为右值而避免拷贝构造
		cond.notify_one();   //唤醒一个进程来执行
	}
}

void ThreadPool::work(int ID)
{
	while (isrunning)
	{
		Task task;
		{
			std::unique_lock<std::mutex> locker(m);
			//如果线程池在运行，且任务列表为空，等待任务
			if (isrunning && tasks.empty())
			{
				cond.wait(locker);
				//std::cout << "This is " << ID << std::endl;
			}
			//如果任务列表不为空，无论线程池是否在运行，都需要执行完任务
			if (!tasks.empty())
			{
				task = tasks.front();
				tasks.pop_front();
			}
		}
		if (task)
		{
			task();
		}
	}
}
