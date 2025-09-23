#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <vector>
#include <chrono>

using namespace std::chrono;

const int MAX_THREADS = 8;




volatile class Bakery {
	bool* flag;
	std::vector<size_t> label;

public:
	Bakery() {}
	void make(const int n) {
		flag = new bool[n];
		label.reserve(n);
		for (int i = 0; i < n; i++) {
			flag[i] = false;
			label.emplace_back(0);
		}

	}

	void destroy() {
		delete[] flag;
		flag = nullptr;
		label.clear();
	}

	void lock(int id) {
		//auto i = std::this_thread::get_id();

		flag[id] = true;
		label[id] = *std::max_element(label.begin(), label.end()) + 1;

		auto k = std::distance(label.begin(), (std::min_element(label.begin(), label.end())));

		while (flag[k] && label[k] < label[id] || (label[k] == label[id] && k < id))
		{
		}

	}
	void unlock(int id) {
		flag[id] = false;
	}

};


volatile int sum{ 0 };

std::mutex m;
std::atomic<int> atomSum{ 0 };
Bakery bake;

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
		bake.lock(threadID);
		sum++;
		bake.unlock(threadID);

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
		bake.lock(threadID);
		atomSum++;
		bake.unlock(threadID);
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
		for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2)
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
		
		for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2)
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
		for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2)
		{
			sum = 0;
			bake.make(num_threads);

			std::thread* threads = new std::thread[num_threads];
			int* threadIDs = new int[num_threads];


			auto timerStart = std::chrono::high_resolution_clock::now();

			for (int i = 0; i < num_threads; ++i)
				threads[i] = std::thread(Add_Bakery, 1'000'0000 / num_threads, i);
			for (int i = 0; i < num_threads; ++i)
				threads[i].join();

			bake.destroy();

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
		for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2)
		{
			atomSum = 0;
			bake.make(num_threads);

			std::thread* threads = new std::thread[num_threads];
			int* threadIDs = new int[num_threads];

			auto timerStart = std::chrono::high_resolution_clock::now();

			for (int i = 0; i < num_threads; ++i)
				threads[i] = std::thread(Add_atomic, 1'000'0000 / num_threads, i);
			for (int i = 0; i < num_threads; ++i)
				threads[i].join();
			bake.destroy();

			delete[] threads;
			delete[] threadIDs;

			auto timerEnd = std::chrono::high_resolution_clock::now();
			auto time_span = duration_cast<milliseconds>(timerEnd - timerStart).count();

			std::cout << "atomSum = " << atomSum << std::endl;
			std::cout << num_threads << "개의 쓰레드 실행 시간 : " << time_span << "ms" << std::endl;
		}
	}
}