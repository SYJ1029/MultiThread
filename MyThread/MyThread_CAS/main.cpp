#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

const int MAX_THREADS = 16;

volatile int sum;

// cas_lock = 0 => 아무도 락을 가지고 있지 않음
//cas_lock = 1 => 누군가 락을 가지고 있음
std::atomic_int cas_lock{ 0 };

std::mutex m;

bool CAS(std::atomic_int *l, int expected, int desired) {
	return std::atomic_compare_exchange_strong(l, &expected, desired);
}

void CAS_LOCK()
{
	
}
void CAS_UNLOCK()
{
}






void worker(int loop)
{
	for (auto i = 0; i < loop; ++i) {
		CAS_LOCK();
		sum += 2;
		CAS_UNLOCK();
	}



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

		for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
			sum = 0;
			auto timerStart = std::chrono::high_resolution_clock::now();

			std::thread* threads = new std::thread[num_threads];


			for (int i = 0; i < num_threads; ++i)
				threads[i] = std::thread(worker, 50000000 / num_threads);
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

	std::cout << std::endl;




	//worker();
	//worker();

}
