#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <atomic>

const int MAX_THREADS = 16;

class NODE
{
public:
	int value;
	NODE* next;
	NODE() {}
	~NODE() {}
	NODE(int input_value) {
		value = input_value;
		next = nullptr;
	}


};

class C_STACK
{
private:
	NODE* top;
	std::mutex stack_lock;
public:

	void clear()
	{
		while (Pop() != -2) {}		
	}

	void Push(int x)
	{
		std::lock_guard<std::mutex> lg(stack_lock);
		NODE* e = new NODE{ x };
		e->next = top;
		top = e;
	}

	int Pop()
	{
		std::lock_guard<std::mutex> lg(stack_lock);
		if (nullptr == top) return -2;
		int temp = top->value;
		NODE* ptr = top;
		top = top->next;
		delete ptr;
		return temp;
	}

	void print20()
	{
		NODE* curr = top->next;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};


C_STACK my_stack;

const int NUM_TEST = 10000000;

void benchmark(const int num_thread)
{
	int key = 0;
	int loop_count = NUM_TEST / num_thread;
	for (auto i = 0; i < loop_count; ++i) {
		if ((rand() % 2 == 0) || (i < 1000))
			my_stack.Push(key++);
		else
			my_stack.Pop();
	}
}

int main()
{
	using namespace std::chrono;

	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		my_stack.clear();
		auto st = high_resolution_clock::now();
		std::vector<std::thread> threads;
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(benchmark, num_threads);
		for (int i = 0; i < num_threads; ++i)
			threads[i].join();
		auto ed = high_resolution_clock::now();
		auto time_span = duration_cast<milliseconds>(ed - st).count();
		std::cout << "Thread Count = " << num_threads << ", Exec Time = " << time_span << "ms.\n";
		std::cout << "Result : ";  my_stack.print20();
	}
}
