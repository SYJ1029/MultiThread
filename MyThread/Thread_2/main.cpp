#include <iostream>
#include <thread>


volatile bool done = false;
std::atomic<int*> bound;
int error;

void worker()
{
	for (int i = 0; i < 5'000'0000; ++i)
	{
		*bound = -(1 + *bound);

	}
	done = true;

}


void monitor()
{
	int error_count = 0;
	while (!done)
	{
		int v = *bound;
		if ((v != 0) && (v != -1))
		{
			++error_count;
			printf("%x\n", v);
			error = v;
		}
	}

	std::cout << "Error count: " << error_count << ", last error: " << error << std::endl;
}

int main()
{
	int value[32];
	long long addr = reinterpret_cast<long long>(&value[16]);
	addr = (addr / 64) * 64;
	addr = addr - 2;
	bound = reinterpret_cast<int*>(addr);
	std::thread t1(worker);
	std::thread t2(monitor);
	t1.join();
	t2.join();

}


