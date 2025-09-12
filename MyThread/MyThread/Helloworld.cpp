#include <iostream>
#include <stdio.h>
#include <thread>
#include <chrono>
#include <mutex>

const int MAX_THREADS = 16;

volatile int sum;

struct NUM
{
	alignas(64) volatile int value;
};

NUM array_sum[MAX_THREADS] = { 0 };

std::mutex m;


void worker3(const int thread_id, const int loop_count)
{
	for (int i = 0; i < loop_count; ++i)
		array_sum[thread_id].value = 
		array_sum[thread_id].value + 2;
}

void worker()
{
	for (auto i = 0; i < 5000000; ++i) 
	{
		//m.lock();
		sum += 2;
		//m.unlock();
	}


}

void worker2(int loop)
{
	volatile int local_sum = 0;
	for (auto i = 0; i < loop; ++i)
	{
		//m.lock();
		local_sum += 2;
		//m.unlock();
	}

	m.lock();
	sum = sum + local_sum;
	m.unlock();
}


void NoLockWorker()
{
	for (auto i = 0; i < 5000000; ++i) 
		sum = sum + 2;
	
}

int main()
{
	using namespace std::chrono;

	{
	
		for (int num_threads = 1; num_threads <= 16; num_threads *= 2){
			sum = 0;
			auto timerStart = std::chrono::high_resolution_clock::now();
			
			std::thread* threads = new std::thread[num_threads];


			for (int i = 0; i < num_threads; ++i) 
				threads[i] = std::thread(worker2, 50000000 / num_threads);
			for (int i = 0; i < num_threads; ++i) 
				threads[i].join();
			

			delete[] threads;

			auto timerEnd = std::chrono::high_resolution_clock::now();
			auto time_span = duration_cast<milliseconds>(timerEnd - timerStart).count();
			
			std::cout << "Sum = " << sum << std::endl;
			std::cout << num_threads << "개의 쓰레드 실행 시간 : " << time_span << "ms" << std::endl;

			//if (num_threads == 1) --num_threads;
		}

	
	

	
	}

	{

		for (int num_threads = 1; num_threads <= 16; num_threads *= 2) {
			sum = 0;
			auto timerStart = std::chrono::high_resolution_clock::now();

			std::thread* threads = new std::thread[num_threads];


			for (int i = 0; i < num_threads; ++i)
				threads[i] = std::thread(worker3, i, 50000000/num_threads);
			for (int i = 0; i < num_threads; ++i) {
				threads[i].join();

				sum += array_sum[i].value;
				array_sum[i].value = 0;
			}
			

			delete[] threads;

			auto timerEnd = std::chrono::high_resolution_clock::now();
			auto time_span = duration_cast<milliseconds>(timerEnd - timerStart).count();

			std::cout << "Sum = " << sum << std::endl;
			std::cout << num_threads << "개의 쓰레드 실행 시간 : " << time_span << "ms" << std::endl;



			//if (num_threads == 1) --num_threads;
		}





	}

	//worker();
	//worker();

}
