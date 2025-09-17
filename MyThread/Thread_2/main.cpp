#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>


std::atomic<bool> g_ready = false;
volatile int g_data = 0;

void Sender()
{
	std::cin >> g_data;
	g_ready = true;
}

void Receiver()
{
	bool expected = false;
	bool desired = true;

	while (g_ready.compare_exchange_weak(expected, desired) == false)
	{

	}
	std::cout << "Data: " << g_data << std::endl;
}

int main()
{
	std::thread t2{ Receiver };
	std::thread t1{ Sender };


	t1.join();
	t2.join();
}