#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <set>
#include <array>
#include <unordered_set>
#include <immintrin.h>

const int MAX_THREADS = 32;
std::mutex wlock;

constexpr int MAX_LEVEL = 9;
volatile int num_threads = 0;
thread_local int thread_id = 0;


class NODE {
public:
	int value;
	NODE* next[MAX_LEVEL + 1];
	int top_level;
	NODE(int v, int top) : value(v)
	{
		for (auto& p : next)p = nullptr;
		top_level = top;
	}
	NODE() : value(-1) {
		for (auto& p : next)p = nullptr;
		top_level = 0;
	}
};


class C_SET {
	NODE* head, * tail;
	std::mutex set_lock;
public:
	C_SET() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = new NODE(-1, MAX_LEVEL);
		tail = new NODE(1000000001, MAX_LEVEL);
		for (auto& p : head->next) p = tail;
	}

	~C_SET() {
		clear();
		delete head;
		delete tail;
	}

	void clear() {
		NODE* curr = head->next[0];
		while (curr != tail) {
			NODE* next = curr->next[0];
			delete curr;
			curr = next;
		}
		for (auto& p : head->next) p = tail;
	}

	void find(NODE* preds[], NODE* succs[], int x)
	{
		preds[MAX_LEVEL] = head;
		for (int level = MAX_LEVEL; level >= 0; level--) {
			if (MAX_LEVEL != level)
				preds[level] = preds[level + 1];
			succs[level] = preds[level]->next[level];
			while (succs[level]->value < x) {
				preds[level] = succs[level];
				succs[level] = preds[level]->next[level];
			}
		}
	}

	bool add(int x)
	{
		NODE* pred[MAX_LEVEL + 1];
		NODE* curr[MAX_LEVEL + 1];
		set_lock.lock();
		find(pred, curr, x);

	
		if (curr[0]->value == x) {
			set_lock.unlock();
			return false;
		}
		else
		{
			int level = 0;
			for (level = 0; level < MAX_LEVEL; ++level)
			{
				if (rand() % 2 == 0)
					break;
			}
			NODE* n = new NODE(x, level);

			for (int i = 0; i <= level; ++i) {
				n->next[i] = curr[i];
				pred[i]->next[i] = n;
			}
			
			set_lock.unlock();
			return true;
		}
	}

	bool remove(int x)
	{
		NODE* pred[MAX_LEVEL + 1];
		NODE* curr[MAX_LEVEL + 1];
		set_lock.lock();
		find(pred, curr, x);

		if (curr[0]->value == x) {
			for (int i = 0; i <= curr[0]->top_level; ++i) {
				pred[i]->next[i] = curr[0]->next[i];
				
			}

			set_lock.unlock();
			delete curr[0];

			return true;
		}
		else
		{
			set_lock.unlock();
			return false;
		}
	}
	bool contains(int x)
	{
		NODE* pred[MAX_LEVEL + 1];
		NODE* curr[MAX_LEVEL + 1];
		set_lock.lock();
		find(pred, curr, x);

		if (curr[0]->value == x) {
			set_lock.unlock();
			return true;
		}


		set_lock.unlock();
		return false;
	}

	void print20()
	{
		NODE* curr = head->next[0];
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next[0])
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};

class SKNODE {
public:
	std::recursive_mutex node_lock;
	int value;
	SKNODE* volatile next[MAX_LEVEL + 1];
	int top_level;
	volatile bool removed = false;
	volatile bool fully_linked = false;
	SKNODE(int v, int top) : value(v), top_level(top), removed(false), fully_linked(false)
	{
		for (auto& p : next)p = nullptr;
	}
	SKNODE() : value(-1), top_level(0), removed(false), fully_linked(false) {
		for (auto& p : next)p = nullptr;
	}
};

class Z_SKLIST
{
	SKNODE* volatile head, * volatile tail;
public:
	Z_SKLIST() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = new SKNODE(-1, MAX_LEVEL);
		tail = new SKNODE(1000000001, MAX_LEVEL);
		for (auto& p : head->next) p = tail;

		head->fully_linked = true;
		tail->fully_linked = true;

	}

	~Z_SKLIST() {
		clear();
		delete head;
		delete tail;
	}

	void clear() {
		SKNODE* curr = head->next[0];
		while (curr != tail) {
			SKNODE* next = curr->next[0];
			delete curr;
			curr = next;
		}
		for (auto& p : head->next) p = tail;
	}

	int find(SKNODE* preds[], SKNODE* succs[], int x)
	{
		int found_level = -1;
		preds[MAX_LEVEL] = head;
		for (int level = MAX_LEVEL; level >= 0; level--) {
			if (MAX_LEVEL != level)
				preds[level] = preds[level + 1];
			succs[level] = preds[level]->next[level];
			while (succs[level]->value < x) {
				preds[level] = succs[level];
				succs[level] = preds[level]->next[level];
			}
			if (found_level == -1 && succs[level]->value == x)
				found_level = level;
		}
		return found_level;
	}

	bool add(int x)
	{
		SKNODE* preds[MAX_LEVEL + 1];
		SKNODE* currs[MAX_LEVEL + 1];

		while (true)
		{
			int f_level = find(preds, currs, x);

			if (f_level != -1)
			{
				SKNODE* node_found = currs[f_level];
				if(node_found->removed == false)
				{
					while (node_found->fully_linked == false) {
					}
					return false;
				}

				continue;
			}

			int highest_locked = -1;

			SKNODE* pred;
			SKNODE* curr;
			volatile bool valid = true;

			int toplevel = 0;
			for (toplevel = 0; toplevel < MAX_LEVEL; ++toplevel)
			{
				if (rand() % 2 == 0)
					break;
			}

			for(int level = 0; valid && level <= toplevel; ++level)
			{
				pred = preds[level];
				curr = currs[level];
				pred->node_lock.lock();
				highest_locked = level;
				valid = (pred->removed == false
					&& curr->removed == false
					&& pred->next[level] == curr);
					
			}
			if (false == valid) {
				for (int i = 0; i <= highest_locked; ++i)
					preds[i]->node_lock.unlock();
				continue;
			}

			
			SKNODE* n = new SKNODE(x, toplevel);

			for (int i = 0; i <= toplevel; ++i) {
				n->next[i] = currs[i];
				preds[i]->next[i] = n;
			}
			n->fully_linked = true;

			for (int i = 0; i <= highest_locked; ++i)
				preds[i]->node_lock.unlock();

			return true;
		}

		
	}

	bool remove(int x)
	{
		SKNODE* preds[MAX_LEVEL + 1];
		SKNODE* currs[MAX_LEVEL + 1];

		int f_level = find(preds, currs, x);
		if (f_level == -1) return false;
		SKNODE* victim = currs[f_level];
		if (victim->top_level != f_level) return false;
		if (victim->removed == true) return false;
		if (victim->fully_linked == false) return false;

		victim->node_lock.lock();
		if (victim->removed == true) {
			victim->node_lock.unlock();
			return false;
		}
		victim->removed = true;

		int top_level = victim->top_level;
		while (true) {

			int highest_locked = 0;
			bool valid = true;
			for (int i = 0; i <= top_level; i++) {
				preds[i]->node_lock.lock();
				highest_locked = i;
				valid = (preds[i]->removed == false)
					&& (preds[i]->next[i] == victim);
				if (false == valid) break;
			}
			if (false == valid) {
				for (int i = 0; i <= highest_locked; i++)
					preds[i]->node_lock.unlock();
				f_level = find(preds, currs, x);
				continue;
			}
			for (int i = top_level; i >= 0; i--)
				preds[i]->next[i] = victim->next[i];
			for (int i = highest_locked; i >= 0; i--)
				preds[i]->node_lock.unlock();
			victim->node_lock.unlock();
			return true;
		}
	}
	bool contains(int x)
	{
		SKNODE* preds[MAX_LEVEL + 1];
		SKNODE* currs[MAX_LEVEL + 1];
		int found_level = find(preds, currs, x);
		return (found_level != -1)
			&& (currs[found_level]->fully_linked == true)
			&& (currs[found_level]->removed == false);
	}

	void print20()
	{
		SKNODE* curr = head->next[0];
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next[0])
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};

class LF_NODE;

class MR {
	volatile long long ptr_and_mark;
public:
	MR(LF_NODE* p)
	{
		ptr_and_mark = reinterpret_cast<long long>(p);
	}
	MR() : ptr_and_mark(0) {}
	LF_NODE* get_ptr()
	{
		long long temp = ptr_and_mark & 0xFFFFFFFFFFFFFFFE;
		return reinterpret_cast<LF_NODE*>(temp);
	}
	bool get_mark() { return (ptr_and_mark & 1) == 1; }
	void set_ptr(LF_NODE* p) {
		ptr_and_mark = reinterpret_cast<long long>(p);
	}
	LF_NODE* get_ptr_and_mark(bool* mark) {
		long long temp = ptr_and_mark;
		*mark = (temp & 1) == 1;
		temp = temp & 0xFFFFFFFFFFFFFFFE;
		return reinterpret_cast<LF_NODE*>(temp);
	}
	bool CAS(LF_NODE* old_ptr, LF_NODE* new_ptr,
		bool old_mark, bool new_mark)
	{
		long long o = reinterpret_cast<long long>(old_ptr);
		if (old_mark) o = o | 1;
		long long n = reinterpret_cast<long long>(new_ptr);
		if (new_mark) n = n | 1;
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<volatile std::atomic<long long>*>(&ptr_and_mark),
			&o, n);
	}
};

class LF_NODE {
public:
	int value;
	MR next[MAX_LEVEL + 1];
	int top_level;
	volatile bool removed = false;
	volatile bool fully_linked = false;

	LF_NODE(int v, int top) : value(v), top_level(top), removed(false), fully_linked(false)
	{
		for (auto& p : next)p = nullptr;
	}
	LF_NODE() : value(-1), top_level(0), removed(false), fully_linked(false) {
		for (auto& p : next)p = nullptr;
	}
};


class LFN_SKLIST
{
	LF_NODE* volatile head, * volatile tail;
public:
	LFN_SKLIST() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = new LF_NODE(-1, MAX_LEVEL);
		tail = new LF_NODE(1000000001, MAX_LEVEL);
		for (auto& p : head->next) p = tail;

		head->fully_linked = true;
		tail->fully_linked = true;

	}

	~LFN_SKLIST() {
		clear();
		delete head;
		delete tail;
	}
	void clear() {
	}
	bool add(int x)
	{
		return true;
	}
	bool remove(int x)
	{

	}
	bool contains(int x)
	{
		return true;
	}
	void print20()
	{
		std::cout << "LFN_SKLIST print20 not implemented.\n";
	}
};

Z_SKLIST clist;

const int NUM_TEST = 4000000;
const int KEY_RANGE = 1000;

class HISTORY {
public:
	int op;
	int i_value;
	bool o_value;
	HISTORY(int o, int i, bool re) : op(o), i_value(i), o_value(re) {}
};

std::array<std::vector<HISTORY>, MAX_THREADS> history;

void check_history(int num_threads)
{
	std::array <int, KEY_RANGE> survive = {};
	std::cout << "Checking Consistency : ";
	if (history[0].size() == 0) {
		std::cout << "No history.\n";
		return;
	}
	for (int i = 0; i < num_threads; ++i) {
		for (auto& op : history[i]) {
			if (false == op.o_value) continue;
			if (op.op == 3) continue;
			if (op.op == 0) survive[op.i_value]++;
			if (op.op == 1) survive[op.i_value]--;
		}
	}
	for (int i = 0; i < KEY_RANGE; ++i) {
		int val = survive[i];
		if (val < 0) {
			std::cout << "ERROR. The value " << i << " removed while it is not in the set.\n";
			exit(-1);
		}
		else if (val > 1) {
			std::cout << "ERROR. The value " << i << " is added while the set already have it.\n";
			exit(-1);
		}
		else if (val == 0) {
			if (clist.contains(i)) {
				std::cout << "ERROR. The value " << i << " should not exists.\n";
				exit(-1);
			}
		}
		else if (val == 1) {
			if (false == clist.contains(i)) {
				std::cout << "ERROR. The value " << i << " shoud exists.\n";
				exit(-1);
			}
		}
	}
	std::cout << " OK\n";
}

void benchmark_check(int num_threads, int th_id)
{
	thread_id = th_id;
	for (int i = 0; i < NUM_TEST / num_threads; ++i) {
		int op = rand() % 3;
		switch (op) {
		case 0: {
			int v = rand() % KEY_RANGE;
			history[th_id].emplace_back(0, v, clist.add(v));
			break;
		}
		case 1: {
			int v = rand() % KEY_RANGE;
			history[th_id].emplace_back(1, v, clist.remove(v));
			break;
		}
		case 2: {
			int v = rand() % KEY_RANGE;
			history[th_id].emplace_back(2, v, clist.contains(v));
			break;
		}
		}
	}
}


void benchmark(const int num_thread, int th_id)
{
	thread_id = th_id;
	const int loop_count = NUM_TEST / num_thread;

	for (int i = 0; i < loop_count; i++) {
		switch (rand() % 3) {
		case 0: {
			int key = rand() % KEY_RANGE;
			clist.add(key);
		}
			  break;
		case 1: {
			int key = rand() % KEY_RANGE;
			clist.remove(key);
		}
			  break;
		case 2: {
			int key = rand() % KEY_RANGE;
			clist.contains(key);
		}
			  break;
		default: std::cout << "Error\n";
			exit(-1);
		}
	}
}

int main()
{
	using namespace std::chrono;

	// 알고리즘 정확성 검사
	std::cout << "Checking for consistency.\n\n";
	{
		for (int i = MAX_THREADS; i >= 1; i = i / 2) {
			num_threads = i;
			std::vector <std::thread> threads;
			clist.clear();
			for (auto& h : history) h.clear();
			auto start_t = system_clock::now();
			for (int j = 0; j < i; ++j)
				threads.emplace_back(benchmark_check, i, j);
			for (auto& th : threads)
				th.join();
			auto end_t = system_clock::now();
			auto exec_t = end_t - start_t;
			auto exec_ms = duration_cast<milliseconds>(exec_t).count();
			std::cout << i << " Threads : SET = ";
			clist.print20();
			std::cout << "Exec time = " << exec_ms << "ms.  ";
			thread_id = 0;
			check_history(i);
		}
	}

	std::cout << "\nBenchmarking for speed.\n\n";
	for (num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		clist.clear();
		auto st = high_resolution_clock::now();
		std::vector<std::thread> threads;
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(benchmark, num_threads, i);
		for (int i = 0; i < num_threads; ++i)
			threads[i].join();
		auto ed = high_resolution_clock::now();
		auto time_span = duration_cast<milliseconds>(ed - st).count();
		std::cout << "Thread Count = " << num_threads << ", Exec Time = " << time_span << "ms.\n";
		std::cout << "Result : ";  clist.print20();
	}





}