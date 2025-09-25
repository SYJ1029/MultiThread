#include "Peterson.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <queue>


Peterson<volatile bool, volatile int> peterson_volatile;
Peterson<std::atomic<bool>, std::atomic<int>> peterson_atomic;
Peterson<std::atomic<bool>, volatile int> peterson_mixed;

volatile int sharedCounter = 0; // 공유 자원

using namespace std::chrono;

void increment(int id, int increments) {
	for (int i = 0; i < increments; ++i) {
		peterson_volatile.lock(id);
		// 임계 구역 시작
		sharedCounter++;
		// 임계 구역 끝
		peterson_volatile.unlock(id);
	}
}

void increment_atomic(int id, int increments) {
	for (int i = 0; i < increments; ++i) {
		peterson_atomic.lock(id);
		// 임계 구역 시작
		sharedCounter++;
		// 임계 구역 끝
		peterson_atomic.unlock(id);
	}
}

void increment_mixed(int id, int increments) {
	for (int i = 0; i < increments; ++i) {
		peterson_mixed.lock(id);
		// 임계 구역 시작
		sharedCounter++;
		// 임계 구역 끝
		peterson_mixed.unlock(id);
	}
}

int main()
{

	const int numIncrements = 1000000;
	{
		std::cout << "<using volatile>" << std::endl << std::endl;
		std::thread t1(increment, 0, numIncrements);
		std::thread t2(increment, 1, numIncrements);

		auto start = std::chrono::high_resolution_clock::now();
		t1.join();
		t2.join();
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = duration_cast<std::chrono::milliseconds>(end - start).count();
		std::cout << "Final counter value: " << sharedCounter << std::endl;
		std::cout << "Time taken: " << duration << " ms" << std::endl;
	}

	{
		sharedCounter = 0; // 카운터 초기화
		std::cout << std::endl << "<using atomic>" << std::endl << std::endl;
		std::thread t1(increment_atomic, 0, numIncrements);
		std::thread t2(increment_atomic, 1, numIncrements);
		auto start = std::chrono::high_resolution_clock::now();
		t1.join();
		t2.join();
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = duration_cast<std::chrono::milliseconds>(end - start).count();
		std::cout << "Final counter value: " << sharedCounter << std::endl;
		std::cout << "Time taken: " << duration << " ms" << std::endl;
	}

	{
		std::cout << std::endl << "<using mixed>" << std::endl << std::endl;
		sharedCounter = 0; // 카운터 초기화
		std::thread t1(increment_mixed, 0, numIncrements);
		std::thread t2(increment_mixed, 1, numIncrements);

		auto start = std::chrono::high_resolution_clock::now();
		t1.join();
		t2.join();
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = duration_cast<std::chrono::milliseconds>(end - start).count();
		std::cout << "Final counter value: " << sharedCounter << std::endl;
		std::cout << "Time taken: " << duration << " ms" << std::endl;
		
	}


}