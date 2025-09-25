#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

using namespace std::chrono;

volatile int victim = 0;
std::atomic <unsigned char> flag[2] = { 0, 0 };
void Lock(int myId)
{
	int other = 1 - myId;
	flag[myId] = 1;
	victim = myId;
	while ((flag[other] == 1) && victim == myId);
}
void Unlock(int myId)
{
	flag[myId] = 0;
}


volatile int sum{ 0 };


void Add(int loopCount, int threadID)
{
	for (int i = 0; i < loopCount; ++i)
	{
		Lock(threadID);
		sum++;
		Unlock(threadID);
	}
}

int main()
{
	{
		std::cout << "Peterson Algorithm" << std::endl << std::endl;
		
		std::thread t1(Add, 1'000'0000 / 2, 0);
		std::thread t2(Add, 1'000'0000 / 2, 1);
		auto start = std::chrono::high_resolution_clock::now();
		t1.join();
		t2.join();
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = duration_cast<std::chrono::milliseconds>(end - start).count();
		std::cout << "Sum = " << sum << std::endl;
		std::cout << "Execution time : " << duration << "ms" << std::endl;

	}
}