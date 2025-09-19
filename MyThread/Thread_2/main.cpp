#include <iostream>
#include <thread>


const int LOOP_COUNT = 5'000'0000;
volatile int x, y;
std::atomic<int> trace_x[LOOP_COUNT], trace_y[LOOP_COUNT];

void thread_x()
{
	for(int i = 0; i < LOOP_COUNT; ++i) {
		x = i;
		trace_x[i] = y;
	}
}

void thread_y()
{
	for(int i = 0; i < LOOP_COUNT; ++i) {
		y = i;
		trace_y[i] = x;
	}
}

int main()
{
	std::thread t0(thread_x);
	std::thread t1(thread_y);

	// Thread 2°³ ½ÇÇà
	int count = 0;
	for (int i = 0; i < LOOP_COUNT; ++i)
		if(trace_x[i] == trace_x[i + 1])
			if (trace_y[trace_x[i]] == trace_y[trace_x[i] + 1]) {
				if (trace_y[trace_x[i]] != i) continue;
				else ++count;
			}
	std::cout << "Total Memory Inconsistency: " << count << std::endl;



	t0.join();
	t1.join();
}

