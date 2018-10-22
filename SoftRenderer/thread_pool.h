//#ifndef MY_THREADPOOL_H
//#define MY_THREADPOOL_H
//
//#include <thread>
//#include <mutex>
//#include <condition_variable>
//#include <memory>
//#include <vector>
//#include <list>
//#include <functional>
//#include <cstdio>
//
//class ThreadPool {
//public:
//	using Task = std::function<void(void)>;
//	explicit ThreadPool(int num);
//	~ThreadPool();
//	ThreadPool(const ThreadPool&) = delete;
//	ThreadPool& operator=(const ThreadPool& rhs) = delete;
//	void append(const Task &task);
//	void append(Task &&task);
//	void start();
//	void stop();
//
//private:
//	bool isrunning;
//	int threadNum;
//	void work();
//	std::mutex m;
//	std::condition_variable cond;
//	std::list<Task> tasks;
//	std::vector<std::shared_ptr<std::thread>> threads;
//};
//
//#endif