#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <vector>
#include <chrono>

using namespace std::chrono;

const int MAX_THREADS = 8;

int num_threads = 1;


class Bakery_volatile {
	volatile bool* flag;
	volatile int* label;

public:
	Bakery_volatile() {
		label = nullptr;
		flag = nullptr;
	}


	void make(int n) {
		flag = new bool[n];
		label = new int[n];
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

		for (int i = 0; i < num_threads; i++) {
			if (label[i] > max) {
				max = label[i];
			}
		}
		return max;
	}

	void lock(volatile int id) {


		flag[id] = true;
		label[id] = MaxLabel() + 1;

		
		for(int i = 0; i < num_threads; ++i)
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

class Bakery_atomic {
	std::atomic<bool>* flag;
	std::atomic<int>* label;
public:
	Bakery_atomic() {
		label = nullptr;
		flag = nullptr;
	}
	void make(int n) {
		flag = new std::atomic<bool>[n];
		label = new std::atomic<int>[n];
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
				//std::cout << num_threads << std::endl;
			}
		}
	}

	void unlock(int id) {
		flag[id] = false;
	}
};


volatile int sum{ 0 };

std::mutex m;
std::atomic<int> atomSum{ 0 };
Bakery_volatile bake1;
Bakery_atomic bake2;

void Add_Default(int loopCount)
{
	volatile int localSum{ 0 };
	for (int i = 0; i < loopCount; ++i)
	{
		//localSum++;
		sum++;
	}
	//sum += localSum;
}

void Add_Lock(int loopCount)
{
	volatile int localSum{ 0 };

	for (int i = 0; i < loopCount; ++i)
	{
		//localSum++;
		m.lock();
		sum++;
		m.unlock();
	}

	//m.lock();
	//sum += localSum;
	//m.unlock();
}

void Add_Bakery(int loopCount, int threadID)
{
	volatile int localSum{ 0 };

	for (int i = 0; i < loopCount; ++i)
	{
		bake1.lock(threadID);
		sum++;
		bake1.unlock(threadID);

		//localSum++;
	}

	//bake.lock(threadID);
	//sum += localSum;
	//bake.unlock(threadID);

}

void Add_atomic(int loopCount, int threadID)
{
	volatile int localSum{ 0 };

	for (int i = 0; i < loopCount; ++i)
	{
		//localSum++;
		bake2.lock(threadID);
		atomSum++;
		bake2.unlock(threadID);
	}

	//bake.lock(threadID);
	//atomSum += localSum;
	//bake.unlock(threadID);
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
		std::cout << "<Mutex 사용>" << std::endl << std::endl;
		
		for (num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2)
		{
			sum = 0;
			std::thread* threads = new std::thread[num_threads];

			auto timerStart = std::chrono::high_resolution_clock::now();

			for (int i = 0; i < num_threads; ++i)
				threads[i] = std::thread(Add_Lock, 1'000'0000 / num_threads);
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
			int* threadIDs = new int[num_threads];


			auto timerStart = std::chrono::high_resolution_clock::now();

			for (int i = 0; i < num_threads; ++i)
				threads[i] = std::thread(Add_Bakery, 1'000'0000 / num_threads, i);
			for (int i = 0; i < num_threads; ++i)
				threads[i].join();

			bake1.destroy();

			delete[] threads;
			delete[] threadIDs;

			auto timerEnd = std::chrono::high_resolution_clock::now();
			auto time_span = duration_cast<milliseconds>(timerEnd - timerStart).count();

			std::cout << "Sum = " << sum << std::endl;
			std::cout << num_threads << "개의 쓰레드 실행 시간 : " << time_span << "ms" << std::endl;
		}

	}

	std::cout << std::endl;


	{
		//Bakery 알고리즘 사용 - atomic
		std::cout << "<Bakery 알고리즘 사용 - atomic>" << std::endl << std::endl;
		for (num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2)
		{
			atomSum = 0;
			bake2.make(num_threads);

			std::thread* threads = new std::thread[num_threads];
			int* threadIDs = new int[num_threads];

			auto timerStart = std::chrono::high_resolution_clock::now();

			for (int i = 0; i < num_threads; ++i)
				threads[i] = std::thread(Add_atomic, 1'000'0000 / num_threads, i);
			for (int i = 0; i < num_threads; ++i)
				threads[i].join();
			bake2.destroy();

			delete[] threads;
			delete[] threadIDs;

			auto timerEnd = std::chrono::high_resolution_clock::now();
			auto time_span = duration_cast<milliseconds>(timerEnd - timerStart).count();

			std::cout << "atomSum = " << atomSum << std::endl;
			std::cout << num_threads << "개의 쓰레드 실행 시간 : " << time_span << "ms" << std::endl;
		}
	}
}