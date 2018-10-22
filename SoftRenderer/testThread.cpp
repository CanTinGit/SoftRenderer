#include "my_thread.h"
#include <iostream>

void func(int n) {
	printf("hello from %d  - \n", n);
}

bool lunxun(int num)
{
	return num;
}


void test() {
	ThreadPool pool(4);
	pool.start();

	for (int i = 0; i < 10; ++i) {
		pool.append(std::bind(func, i));
	}
	bool hasFinished = false;
	while (hasFinished == false)
	{
		//std::cout << "开始一次查询 **************" << std::endl;
		hasFinished = pool.finishTask();
		//std::cout << hasFinished << std::endl;

	}
}

//int main() {
//	test();
//	int i;
//	std::cin >> i;
//	//int k = 100;
//	//while(k>0)
//	//{
//	//	
//	//}
//}