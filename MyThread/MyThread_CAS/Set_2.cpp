#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

const int MAX_THREADS = 16;

class Node
{
public:
	int value;
	Node* next;
	Node(int val) : value(val), next(nullptr) {}

	void lock() { nodeLock.lock(); }
	void unlock() { nodeLock.unlock(); }

private:
	std::mutex nodeLock;
};





C_SET clist;

const auto NUM_TEST = 4000000;
const auto KEY_RANGE = 1000;



void Benchmark(const int num_thread)
{
	const int loop_count = NUM_TEST / num_thread;

	int key{ 0 };

	for (int i = 0; i < loop_count; i++) {
		switch (rand() % 3) {
		case 0:
			key = rand() % KEY_RANGE;
			clist.add(key);
			break;
		case 1:
			key = rand() % KEY_RANGE;
			clist.remove(key);
			break;
		case 2:
			key = rand() % KEY_RANGE;
			clist.contains(key);
			break;
		default:
			std::cout << "Error\n";
			exit(-1);
		}
	}
}

int main()
{
	std::cout << "<C_SET 테스트>\n\n";

	using namespace std::chrono;
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2)
	{
		clist.clear();
		auto start = high_resolution_clock::now();
		std::vector<std::thread> threads;
		for (int i = 0; i < num_threads; i++)
			threads.emplace_back(Benchmark, num_threads);
		for (auto& th : threads)
			th.join();
		auto end = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end - start).count();
		std::cout << num_threads << "개의 쓰레드 실행 시간 : " << duration << "ms" << std::endl;
		std::cout << "20개 출력 결과: "; clist.print20();
	}

}