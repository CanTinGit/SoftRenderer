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
//	//线程池关闭，并通知所有线程可以取任务了
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
////添加任务，参数为左值
//void ThreadPool::append(const Task &task)
//{
//	if (isrunning) {
//		std::unique_lock<std::mutex> locker(m);
//		tasks.push_back(task);
//		cond.notify_one();
//	}
//}
////添加任务，参数为一个右值
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
//			//如果线程池在运行，且任务列表为空，等待任务
//			if (isrunning && tasks.empty())
//			{
//				cond.wait(locker);
//			}
//			//如果任务列表不为空，无论线程池是否在运行，都需要执行完任务
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