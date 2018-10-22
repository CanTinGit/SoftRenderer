//#include "thread_pool.h"
//
//ThreadPool::ThreadPool(int num) : threadNum(num), isrunning(false), m()
//{
//}
//
//ThreadPool::~ThreadPool()
//{
//	if (isrunning)
//	{
//		stop();
//	}
//}
//
//void ThreadPool::start()
//{
//	isrunning = true;
//	threads.reserve(threadNum);
//	for (int i = 0; i < threadNum; ++i)
//	{
//		threads.push_back(std::make_shared<std::thread>(&ThreadPool::work, this));
//	}
//}
//
//void ThreadPool::stop()
//{
//	//�̳߳عرգ���֪ͨ�����߳̿���ȡ������
//	{
//		std::unique_lock<std::mutex> locker(m);
//		isrunning = false;
//		cond.notify_all();
//	}
//	for (int i = 0; i < threads.size(); ++i)
//	{
//		auto t = threads[i];
//		if (t->joinable())
//			t->join();
//	}
//}
////������񣬲���Ϊ��ֵ
//void ThreadPool::append(const Task &task)
//{
//	if (isrunning) {
//		std::unique_lock<std::mutex> locker(m);
//		tasks.push_back(task);
//		cond.notify_one();
//	}
//}
////������񣬲���Ϊһ����ֵ
//void ThreadPool::append(Task &&task)
//{
//	if (isrunning) {
//		std::unique_lock<std::mutex> locker(m);
//		tasks.push_back(std::move(task));
//		cond.notify_one();
//	}
//
//}
//
//void ThreadPool::work()
//{
//	while (isrunning)
//	{
//		Task task;
//		{
//			std::unique_lock<std::mutex> locker(m);
//			//����̳߳������У��������б�Ϊ�գ��ȴ�����
//			if (isrunning && tasks.empty())
//			{
//				cond.wait(locker);
//			}
//			//��������б�Ϊ�գ������̳߳��Ƿ������У�����Ҫִ��������
//			if (!tasks.empty())
//			{
//				task = tasks.front();
//				tasks.pop_front();
//			}
//		}
//		if (task)
//			task();
//	}
//}