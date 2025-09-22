#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <vector>

int threadID = 1;

volatile int sum{ 0 };

std::mutex m;
std::atomic<int> atomSum{ 0 };
Bakery bake;

void Add_Lock(int loopCount)
{
	int localSum{ 0 };

	for (int i = 0; i < loopCount; ++i)
	{
		localSum++;
	}

	m.lock();
	sum += localSum;
	m.unlock();
}

void Add_Bakery(int loopCount)
{

	int id = threadID++;

	int localSum{ 0 };

	for (int i = 0; i < loopCount; ++i)
	{
		localSum++;
	}

	
}

void Add_atomic(int loopCount)
{
	int localSum{ 0 };

	for (int i = 0; i < loopCount; ++i)
	{
		localSum++;
	}

	atomSum += localSum;
}


struct Lock
{
	void lock() {}
	void unlock() {}
};




class Bakery {
	bool* flag;
	std::vector<size_t> label;

public:
	Bakery() {}
	void make(int n) {
		flag = new bool[n];
		label.reserve(n);
		for (int i = 0; i < n; i++) {
			flag[i] = false;
			label.emplace_back(0);
		}

	}
	void lock(int id) {
		auto i = std::this_thread::get_id();

		flag[id] = true;

		label[id] = *std::max_element(label.begin(), label.end()) + 1;
		auto k = std::distance(label.begin(), (std::min_element(label.begin(), label.end())));
		while (true)
		{
			if (id != k && flag[k] && label[k] < label[id] && k < id)
				break;
		}

		while (k != id && flag[k] && label[k] < label[id] && k < id) {}

	}
	void unlock(int id) {
		flag[id] = false;

	}

};



int main()
{
	{
		// volatile만 사용
	}

	{
		// mutex 사용
	}

	{
		// atomic 사용
	}

	{
		// Bakery 알고리즘 사용
		

	}
}