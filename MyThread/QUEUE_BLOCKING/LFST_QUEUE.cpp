#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <atomic>

const int MAX_THREADS = 16;

class NODE {
public:
	int value;
	NODE* volatile next;
	NODE(int v) : value(v), next(nullptr) {}
};

class DUMMY_MUTEX {
public:
	void lock() {}
	void unlock() {}
};

class C_QUEUE {
	NODE* head, * tail;
	std::mutex set_lock;
public:
	C_QUEUE() {
		head = tail = new NODE(-1);
	}

	~C_QUEUE() {
		clear();
		delete head;
	}

	void clear() {
		NODE* curr = head->next;
		while (nullptr != curr) {
			NODE* next = curr->next;
			delete curr;
			curr = next;
		}
		tail = head;
		head->next = nullptr;
	}

	void enqueue(int x)
	{
		NODE* new_node = new NODE(x);
		set_lock.lock();
		tail->next = new_node;
		tail = new_node;
		set_lock.unlock();
	}

	int dequeue()
	{
		NODE* temp;
		set_lock.lock();
		if (nullptr == head->next) {
			set_lock.unlock();
			return -1;
		}
		int res = head->next->value;
		temp = head;
		head = head->next;
		set_lock.unlock();
		delete temp;
		return res;
	}

	void print20()
	{
		NODE* curr = head->next;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};

class LF_QUEUE {
	NODE* volatile head, * volatile tail;
public:
	LF_QUEUE() {
		head = tail = new NODE(-1);
	}

	~LF_QUEUE() {
		clear();
		delete head;
	}

	void clear() {
		NODE* curr = head->next;
		while (nullptr != curr) {
			NODE* next = curr->next;
			delete curr;
			curr = next;
		}
		tail = head;
		head->next = nullptr;
	}

	bool CAS(NODE* volatile* addr, NODE* expected, NODE* new_value)
	{
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<volatile std::atomic<NODE*>*>(addr),
			&expected,
			new_value);
	}

	void enqueue(int x)
	{
		NODE* new_node = new NODE(x);
		while (true) {
			NODE* old_tail = tail;
			NODE* old_next = old_tail->next;
			if (old_tail != tail)
				continue;
			if (old_next == nullptr) {
				if (true == CAS(&old_tail->next, nullptr, new_node)) {
					CAS(&tail, old_tail, new_node);
					return;
				}
			}
			else
				CAS(&tail, old_tail, old_next);

		}
	}

	int dequeue()
	{
		while (true) {
			NODE* old_head = head;
			NODE* old_next = old_head->next;
			NODE* old_tail = tail;
			if (old_head != head)
				continue;
			if (old_next == nullptr)
				return -1;
			if (old_tail == old_head) {
				CAS(&tail, old_tail, old_next);
				continue;
			}
			int res = old_next->value;
			if (true == CAS(&head, old_head, old_next)) {
				delete old_head;
				return res;
			}
		}
	}

	void print20()
	{
		NODE* curr = head->next;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};

class STNODE;
class STAMPED_PTR {
public:
	std::atomic<long long> combined;
	void set_ptr(STNODE* p) {
		long long ptr_val = reinterpret_cast<long long>(p);
		combined = ptr_val << 32;
	}
	STNODE* get_ptr() {
		return reinterpret_cast<STNODE*>(combined >> 32);
	}
	int get_stamp() {
		return static_cast<int>(combined & 0xFFFFFFFF);
	}
};

class STNODE {
public:
	int value;
	STAMPED_PTR next;
	STNODE(int v) : value(v) {}
};

class LFST_QUEUE {
	STAMPED_PTR head, tail;
public:
	LFST_QUEUE() {
		STNODE* g = new STNODE(-1);
		head.set_ptr(g);
		tail.set_ptr(g);
	}

	~LFST_QUEUE() {
		clear();
		delete head.get_ptr();


	}

	void clear() {
		STNODE* curr = head.get_ptr()->next.get_ptr();
		while (nullptr != curr) {
			STNODE* next = curr->next.get_ptr();

			delete curr;
			curr = next;
		}

		STNODE* g = new STNODE(-1);
		head.set_ptr(g);
		tail.set_ptr(g);
	}

	bool CAS(STAMPED_PTR* addr, STNODE* expected,
		STNODE* new_value, int old_stamp, int new_stamp)
	{
		volatile long long* c_addr = reinterpret_cast<volatile long long*>(addr);
		long long expected_combined = (static_cast<long long>(reinterpret_cast<long long>(expected)) << 32) | old_stamp;
		long long new_combined = (static_cast<long long>(reinterpret_cast<long long>(new_value)) << 32) | new_stamp;
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<volatile std::atomic<long long>*>(c_addr),
			&expected_combined,
			new_combined);
	}

	void enqueue(int x)
	{
		STNODE* new_node = new STNODE(x);
		while (true) {
			STAMPED_PTR old_tail; old_tail.combined = tail.combined.load();
			STAMPED_PTR old_next; old_next.combined = old_tail.get_ptr()->next.combined.load();
			if (old_tail.get_ptr() != tail.get_ptr())
				continue;
			if (old_next.get_ptr() == nullptr) {
				if (true == CAS(&old_tail.get_ptr()->next, nullptr, new_node,
					old_next.get_stamp(), old_next.get_stamp() + 1)) {
					CAS(&tail, old_tail.get_ptr(), new_node,
						old_tail.get_stamp(), old_tail.get_stamp() + 1);
					return;
				}
			}
			else
				CAS(&tail, old_tail.get_ptr(), old_next.get_ptr(),
					old_tail.get_stamp(), old_tail.get_stamp() + 1
				);
		}
	}

	int dequeue()
	{
		while (true) {
			STAMPED_PTR old_head; old_head.combined = head.combined.load();
			STAMPED_PTR old_next; old_next.combined = old_head.get_ptr()->next.combined.load();
			STAMPED_PTR old_tail; old_tail.combined = tail.combined.load();
			if (old_head.get_ptr() != head.get_ptr())
				continue;
			if (old_next.get_ptr() == nullptr)
				return -1;
			if (old_tail.get_ptr() == old_head.get_ptr()) {
				CAS(&tail, old_tail.get_ptr(), old_next.get_ptr(),
					old_tail.get_stamp(), old_tail.get_stamp() + 1);
				continue;
			}
			int res = old_next.get_ptr()->value;
			if (true == CAS(&head, old_head.get_ptr(), old_next.get_ptr(),
				old_head.get_stamp(), old_head.get_stamp() + 1)) {
				delete old_head.get_ptr();
				return res;
			}
		}
	}

	void print20()
	{
		STNODE* curr = head.get_ptr()->next.get_ptr();
		for (int i = 0; i < 20 && curr != nullptr;
			i++, curr = curr->next.get_ptr())
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};

C_QUEUE my_queue;

const int NUM_TEST = 10000000;

void benchmark(const int num_thread, int th_id)
{
	const int loop_count = NUM_TEST / num_thread;

	int key = 0;
	for (int i = 0; i < loop_count; i++) {
		if ((i < 32) || (rand() % 2 == 0))
			my_queue.enqueue(key++);
		else
			my_queue.dequeue();
	}

}

int main()
{
	using namespace std::chrono;

	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		my_queue.clear();
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
