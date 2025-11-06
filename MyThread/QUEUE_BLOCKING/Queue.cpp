#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <array>
#include <set>

const int MAX_THREADS = 32;

volatile int num_threads = 0;

class NODE {
public:
	int value;
	NODE* next;
	std::mutex node_lock;
	bool removed;
	NODE(int v) : value(v), next(nullptr), removed(false) {}
	void lock() { node_lock.lock(); }
	void unlock() { node_lock.unlock(); }
};

class Queue
{
private:
	std::mutex enqLock;
	std::mutex deqLock;

	NODE* head{nullptr};
	NODE* tail{nullptr};
	
public:
	Queue() {

		//head->next = tail;

	}
	~Queue() {

		

	}

	void clear()
	{
		while (head->next)
		{
			NODE* temp = tail;
			tail = tail->next;
			delete temp;
		}
	}

	void Enqueue(int key)
	{
		std::lock_guard<std::mutex> enqLg(enqLock);

		NODE* e = new NODE(key);
		if (tail) 
		{
			
			tail->next = e;
			tail = e;
		}
		else if (head) 
		{
			tail = e;
			head->next = tail;
		}
		else
		{
			head = e;
		}

	}

	int Dequeue()
	{
		std::lock_guard<std::mutex> enqLg(deqLock);

		if (head->next)
		{
			NODE* result = head->next;
			head = head->next;

			return result->value;
		}
		else
		{
			return -1;
		}
	}

	void print20()
	{
		NODE* curr = head->next;
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};


const int NUM_TEST = 4000000;
const int KEY_RANGE = 1000;



Queue my_queue;


volatile int key = 0;

void benchmark(int num_thread, int th_id) {
	for (int i = 0; i < NUM_TEST / num_thread; i++) {
		if ((i < 32) || (rand() % 2 == 0))
			my_queue.Enqueue(key++);
		else
			my_queue.Dequeue();
	}
}


int main()
{
	using namespace std::chrono;



	std::cout << "\nBenchmarking for speed.\n\n";
	for (num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		//my_queue.clear();
		key = 0;
		auto st = high_resolution_clock::now();
		std::vector<std::thread> threads;
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(benchmark, num_threads, i);
		for (int i = 0; i < num_threads; ++i)
			threads[i].join();
		auto ed = high_resolution_clock::now();
		auto time_span = duration_cast<milliseconds>(ed - st).count();
		std::cout << "Thread Count = " << num_threads << ", Exec Time = " << time_span << "ms.\n";
		std::cout << "Result : ";  my_queue.print20();
	}
}