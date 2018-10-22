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


//������񣬲���Ϊ��ֵ
void ThreadPool::append(const Task& task)
{
	if (isrunning)
	{
		std::unique_lock<std::mutex> locker(m);
		tasks.push_back(task);
		cond.notify_one();   //����һ��������ִ��
	}
}

//���������ֵ����
void ThreadPool::append(Task&& task)
{
	if (isrunning)
	{
		std::unique_lock<std::mutex> locker(m);
		tasks.push_back(std::move(task));       //std::move�����Խ���ֵ��Ϊ��ֵ�����⿽������
		cond.notify_one();   //����һ��������ִ��
	}
}

void ThreadPool::work(int ID)
{
	while (isrunning)
	{
		Task task;
		{
			std::unique_lock<std::mutex> locker(m);
			//����̳߳������У��������б�Ϊ�գ��ȴ�����
			if (isrunning && tasks.empty())
			{
				cond.wait(locker);
				//std::cout << "This is " << ID << std::endl;
			}
			//��������б�Ϊ�գ������̳߳��Ƿ������У�����Ҫִ��������
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
