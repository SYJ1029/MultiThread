#include <iostream>
#include <algorithm>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

using namespace std::chrono;
const int MAX_THREADS = 8;
int num_threads = 1;


template <typename T, typename Y>
class Bakery {
	T* flag;
	Y* label;
public:
	Bakery() {
		label = nullptr;
		flag = nullptr;
	}
	void make(int n) {
		flag = new T[n];
		label = new Y[n];
		for (int i = 0; i < n; i++) {
			flag[i] = false;
			label[i] = 0;
		}
	}
	void destroy() {
		delete[] flag;
		delete[] label;
		flag = nullptr;
		label = nullptr;
	}
	int MaxLabel() {
		int max = label[0];
		for (int i = 1; i < num_threads; i++) {
			if (label[i] > max) {
				max = label[i];
			}
		}
		return max;
	}
	void lock(int id) {
		flag[id] = true;
		label[id] = MaxLabel() + 1;
		for (int i = 0; i < num_threads; ++i)
		{
			if (i == id) continue;
			while (flag[i] && (label[i] < label[id] || (label[i] == label[id] && i < id)))
			{
			}
		}
	}
	void unlock(int id) {
		flag[id] = false;
		label[id] = 0;
	}
};


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


