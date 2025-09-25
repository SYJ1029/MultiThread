#include <iostream>
#include <algorithm>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include "Bakery.h"

using namespace std::chrono;




volatile int sum{ 0 };

std::atomic<int> atomSum{ 0 };
std::mutex m;

Bakery<volatile bool, volatile int> bake1;
Bakery<std::atomic<bool>, std::atomic<int>> bake2;

void Add_Default(int loopCount)
{
	volatile int localSum{ 0 };
	for (int i = 0; i < loopCount; ++i)
	{
		sum++;
	}
	//sum += localSum;
}

void Add_Mutex(int loopCount)
{
	volatile int localSum{ 0 };
	for (int i = 0; i < loopCount; ++i)
	{
		m.lock();
		sum++;
		m.unlock();
	}

}

void Add_Bakery(int loopCount, int threadID)
{
	volatile int localSum{ 0 };
	for (int i = 0; i < loopCount; ++i)
	{
		bake1.lock(threadID);
		sum++;
		bake1.unlock(threadID);
	}

}

void Add_atomic(int loopCount, int threadID)
{
	volatile int localSum{ 0 };
	for (int i = 0; i < loopCount; ++i)
	{
		bake2.lock(threadID);
		sum++;
		bake2.unlock(threadID);
	}
}

int main()
{
	{
		// volatile만 사용
		std::cout << "<volatile만 사용>" << std::endl << std::endl;
		for (num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2)
		{
			sum = 0;
			std::thread* threads = new std::thread[num_threads];
			auto timerStart = std::chrono::high_resolution_clock::now();
			for (int i = 0; i < num_threads; ++i)
				threads[i] = std::thread(Add_Default, 1'000'0000 / num_threads);
			for (int i = 0; i < num_threads; ++i)
				threads[i].join();
			delete[] threads;
			auto timerEnd = std::chrono::high_resolution_clock::now();
			auto time_span = duration_cast<milliseconds>(timerEnd - timerStart).count();
			std::cout << "Sum = " << sum << std::endl;
			std::cout << num_threads << "개의 쓰레드 실행 시간 : " << time_span << "ms" << std::endl;
		}
	}

	std::cout << std::endl;

	{
		// mutex 사용
		std::cout << "<mutex 사용>" << std::endl << std::endl;
		for (num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2)
		{
			sum = 0;
			std::thread* threads = new std::thread[num_threads];
			auto timerStart = std::chrono::high_resolution_clock::now();
			for (int i = 0; i < num_threads; ++i)
				threads[i] = std::thread(Add_Mutex, 1'000'0000 / num_threads);
			for (int i = 0; i < num_threads; ++i)
				threads[i].join();
			delete[] threads;
			auto timerEnd = std::chrono::high_resolution_clock::now();
			auto time_span = duration_cast<milliseconds>(timerEnd - timerStart).count();
			std::cout << "Sum = " << sum << std::endl;
			std::cout << num_threads << "개의 쓰레드 실행 시간 : " << time_span << "ms" << std::endl;
		}
	}

	std::cout << std::endl;

	{
		// Bakery 알고리즘 사용 - volatile
		std::cout << "<Bakery 알고리즘 사용 - volatile>" << std::endl << std::endl;
		for (num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2)
		{
			sum = 0;
			bake1.make(num_threads);
			std::thread* threads = new std::thread[num_threads];
			auto timerStart = std::chrono::high_resolution_clock::now();
			for (int i = 0; i < num_threads; ++i)
				threads[i] = std::thread(Add_Bakery, 1'000'0000 / num_threads, i);
			for (int i = 0; i < num_threads; ++i)
				threads[i].join();
			bake1.destroy();
			delete[] threads;
			auto timerEnd = std::chrono::high_resolution_clock::now();
			auto time_span = duration_cast<milliseconds>(timerEnd - timerStart).count();
			std::cout << "Sum = " << sum << std::endl;
			std::cout << num_threads << "개의 쓰레드 실행 시간 : " << time_span << "ms" << std::endl;
		}
	}

	std::cout << std::endl;

	{
		// Bakery 알고리즘 사용 - atomic
		std::cout << "<Bakery 알고리즘 사용 - atomic>" << std::endl << std::endl;
		for (num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2)
		{
			sum = 0;
			bake2.make(num_threads);
			std::thread* threads = new std::thread[num_threads];
			auto timerStart = std::chrono::high_resolution_clock::now();
			for (int i = 0; i < num_threads; ++i)
				threads[i] = std::thread(Add_atomic, 1'000'0000 / num_threads, i);
			for (int i = 0; i < num_threads; ++i)
				threads[i].join();
			bake2.destroy();
			delete[] threads;
			auto timerEnd = std::chrono::high_resolution_clock::now();
			auto time_span = duration_cast<milliseconds>(timerEnd - timerStart).count();
			std::cout << "Sum = " << sum << std::endl;
			std::cout << num_threads << "개의 쓰레드 실행 시간 : " << time_span << "ms" << std::endl;
		}
	}

}


