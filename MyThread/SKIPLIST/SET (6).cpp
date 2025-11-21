#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <array>
#include <set>

const int MAX_THREADS = 16;

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

class DUMMY_MUTEX {
	public:
		void lock() {}
		void unlock() {}	
};

class C_SET {
	NODE* head, * tail;
	DUMMY_MUTEX set_lock;
public:
	C_SET() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = new NODE(-1);
		tail = new NODE(1000000001);
		head->next = tail;
	}

	~C_SET() {
		clear();
		delete head;
		delete tail;
	}

	void clear() {
		NODE* curr = head->next;
		while (curr != tail) {
			NODE* next = curr->next;
			delete curr;
			curr = next;
		}
		head->next = tail;
	}

	bool add(int x)
	{
		NODE* pred = head;
		set_lock.lock();
		NODE* curr = head->next;
		while (curr->value < x) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->value == x) {
			set_lock.unlock();
			return false;
		}
		else
		{
			auto n = new NODE(x);
			n->next = curr;
			pred->next = n;
			set_lock.unlock();
			return true;
		}
	}

	bool remove(int x)
	{
		NODE* pred = head;
		set_lock.lock();
		NODE* curr = head->next;
		while (curr->value < x) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->value == x) {
			pred->next = curr->next;
			set_lock.unlock();
			delete curr;
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
		NODE* pred = head;
		set_lock.lock();
		NODE* curr = head->next;
		while (curr->value < x) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->value == x) {
			set_lock.unlock();
			return true;
		}
		else
		{
			set_lock.unlock();	
			return false;
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

class F_SET {
	NODE* head, * tail;
public:
	F_SET() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = new NODE(-1);
		tail = new NODE(1000000001);
		head->next = tail;
	}

	~F_SET() {
		clear();
		delete head;
		delete tail;
	}

	void clear() {
		NODE* curr = head->next;
		while (curr != tail) {
			NODE* next = curr->next;
			delete curr;
			curr = next;
		}
		head->next = tail;
	}

	bool add(int x)
	{
		NODE* pred = head;
		pred->lock();
		NODE* curr = pred->next;
		curr->lock();
		while (curr->value < x) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->value == x) {
			pred->unlock();
			curr->unlock();
			return false;
		}
		else
		{
			auto n = new NODE(x);
			n->next = curr;
			pred->next = n;
			pred->unlock();
			curr->unlock();
			return true;
		}
	}

	bool remove(int x)
	{
		NODE* pred = head;
		pred->lock();
		NODE* curr = pred->next;
		curr->lock();
		while (curr->value < x) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->value == x) {
			pred->next = curr->next;
			pred->unlock();
			curr->unlock();
			delete curr;
			return true;
		}
		else
		{
			pred->unlock();
			curr->unlock();
			return false;
		}
	}
	bool contains(int x)
	{
		NODE* pred = head;
		pred->lock();
		NODE* curr = pred->next;
		curr->lock();
		while (curr->value < x) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->value == x) {
			pred->unlock();
			curr->unlock();
			return true;
		}
		else
		{
			pred->unlock();
			curr->unlock();
			return false;
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

class L_SET {
	NODE* head, * tail;
public:
	L_SET() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = new NODE(-1);
		tail = new NODE(1000000001);
		head->next = tail;
	}

	~L_SET() {
		clear();
		delete head;
		delete tail;
	}

	void clear() {
		NODE* curr = head->next;
		while (curr != tail) {
			NODE* next = curr->next;
			delete curr;
			curr = next;
		}
		head->next = tail;
	}

	bool validate(NODE* pred, NODE* curr) {
		return (pred->removed == false) 
			&& (curr->removed == false) 
			&& (pred->next == curr);
	}

	bool add(int x)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;
			while (curr->value < x) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();

			if (true == validate(pred, curr)) {

				if (curr->value == x) {
					pred->unlock();
					curr->unlock();
					return false;
				}
				else
				{
					auto n = new NODE(x);
					n->next = curr;
					pred->next = n;
					pred->unlock();
					curr->unlock();
					return true;
				}
			}

			pred->unlock();
			curr->unlock();
		}
	}

	bool remove(int x)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;
			while (curr->value < x) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();
			if (false == validate(pred, curr)) {
				pred->unlock(); curr->unlock();
				continue;
			}

			if (curr->value == x) {
				curr->removed = true;
				pred->next = curr->next;
				pred->unlock();
				curr->unlock();
				//delete curr;
				return true;
			}
			else
			{
				pred->unlock();
				curr->unlock();
				return false;
			}
		}
	}
	bool contains(int x)
	{
		while (true) {
			NODE* curr = head->next;
			while (curr->value < x)
				curr = curr->next;
			return (curr->value == x) && (curr->removed == false);
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

class NODE_SP {
public:
	int value;
	std::shared_ptr<NODE_SP> next;
	std::mutex node_lock;
	bool removed;
	NODE_SP(int v) : value(v), next(nullptr), removed(false) {}
	void lock() { node_lock.lock(); }
	void unlock() { node_lock.unlock(); }
};

//class L_SET_SP {
//	std::shared_ptr<NODE_SP> head, tail;
//public:
//	L_SET_SP() {
//		/* 값을 0부터 1000까지로 제한하겠음 */
//		head = std::make_shared<NODE_SP>(-1);
//		tail = std::make_shared<NODE_SP>(1000000001);
//		head->next = tail;
//	}
//
//	~L_SET_SP() {
//	}
//
//	void clear() {
//		head->next = tail;
//	}
//
//	bool validate(const std::shared_ptr<NODE_SP> &pred, 
//		const std::shared_ptr<NODE_SP> &curr) {
//		return (pred->removed == false)
//			&& (curr->removed == false)
//			&& (std::atomic_load(&pred->next) == curr);
//	}
//
//	bool add(int x)
//	{
//		while (true) {
//			auto pred = head;
//			auto curr = std::atomic_load(&pred->next);
//			while (curr->value < x) {
//				pred = curr;
//				curr = std::atomic_load(& curr->next);
//			}
//
//			pred->lock(); curr->lock();
//
//			if (true == validate(pred, curr)) {
//
//				if (curr->value == x) {
//					pred->unlock();
//					curr->unlock();
//					return false;
//				}
//				else
//				{
//					auto n = std::make_shared<NODE_SP>(x);
//					n->next = curr;
//					std::atomic_exchange(&pred->next, n);
//					pred->unlock();
//					curr->unlock();
//					return true;
//				}
//			}
//
//			pred->unlock();
//			curr->unlock();
//		}
//	}
//
//	bool remove(int x)
//	{
//		while (true) {
//			auto pred = head;
//			auto curr = std::atomic_load(&pred->next);
//			while (curr->value < x) {
//				pred = curr;
//				curr = std::atomic_load(&curr->next);
//			}
//
//			pred->lock(); curr->lock();
//			if (false == validate(pred, curr)) {
//				pred->unlock(); curr->unlock();
//				continue;
//			}
//
//			if (curr->value == x) {
//				curr->removed = true;
//				std::atomic_exchange(&pred->next, 
//					std::atomic_load(& curr->next));
//				pred->unlock();
//				curr->unlock();
//				//delete curr;
//				return true;
//			}
//			else
//			{
//				pred->unlock();
//				curr->unlock();
//				return false;
//			}
//		}
//	}
//	bool contains(int x)
//	{
//		while (true) {
//			auto curr = std::atomic_load(&head->next);
//			while (curr->value < x)
//				curr = std::atomic_load(&curr->next);
//			return (curr->value == x) && (curr->removed == false);
//		}
//	}
//
//	void print20()
//	{
//		auto curr = head->next;
//		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next)
//			std::cout << curr->value << ", ";
//		std::cout << "\n";
//	}
//};

class NODE_SPA {
public:
	int value;
	std::atomic<std::shared_ptr<NODE_SPA>> next;
	std::mutex node_lock;
	bool removed;
	NODE_SPA(int v) : value(v), next(nullptr), removed(false) {}
	void lock() { node_lock.lock(); }
	void unlock() { node_lock.unlock(); }
};

class L_SET_SPA {
	std::shared_ptr<NODE_SPA> head, tail;
public:
	L_SET_SPA() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = std::make_shared<NODE_SPA>(-1);
		tail = std::make_shared<NODE_SPA>(1000000001);
		head->next = tail;
	}

	~L_SET_SPA() {
	}

	void clear() {
		head->next = tail;
	}

	bool validate(const std::shared_ptr<NODE_SPA>& pred,
		const std::shared_ptr<NODE_SPA>& curr) {
		return (pred->removed == false)
			&& (curr->removed == false)
			&& (pred->next.load() == curr);
	}

	bool add(int x)
	{
		while (true) {
			auto pred = head;
			std::shared_ptr<NODE_SPA>curr = pred->next;
			while (curr->value < x) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();

			if (true == validate(pred, curr)) {

				if (curr->value == x) {
					pred->unlock();
					curr->unlock();
					return false;
				}
				else
				{
					auto n = std::make_shared<NODE_SPA>(x);
					n->next = curr;
					pred->next = n;
					pred->unlock();
					curr->unlock();
					return true;
				}
			}

			pred->unlock();
			curr->unlock();
		}
	}

	bool remove(int x)
	{
		while (true) {
			auto pred = head;
			std::shared_ptr<NODE_SPA> curr = pred->next;
			while (curr->value < x) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();
			if (false == validate(pred, curr)) {
				pred->unlock(); curr->unlock();
				continue;
			}

			if (curr->value == x) {
				curr->removed = true;
				pred->next.store(curr->next);
				pred->unlock();
				curr->unlock();
				//delete curr;
				return true;
			}
			else
			{
				pred->unlock();
				curr->unlock();
				return false;
			}
		}
	}
	bool contains(int x)
	{
		while (true) {
			std::shared_ptr<NODE_SPA> curr = head->next;
			while (curr->value < x)
				curr = curr->next;
			return (curr->value == x) && (curr->removed == false);
		}
	}

	void print20()
	{
		std::shared_ptr<NODE_SPA> curr = head->next;
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};


#include <queue>

std::queue<NODE*> node_pool;
std::mutex pool_lock;

void recycle_nodes() {
	while (false == node_pool.empty()) {
		auto n = node_pool.front();
		node_pool.pop();
		delete n;
	}
}

NODE* get_new_node(int v) {
	return new NODE(v);
}

void delete_node(NODE* n) {
	std::lock_guard<std::mutex> lg(pool_lock);
	node_pool.push(n);
}

class L_SET_NML {
	NODE* head, * tail;
public:
	L_SET_NML() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = new NODE(-1);
		tail = new NODE(1000000001);
		head->next = tail;
	}

	~L_SET_NML() {
		clear();
		delete head;
		delete tail;
	}

	void clear() {
		NODE* curr = head->next;
		while (curr != tail) {
			NODE* next = curr->next;
			delete curr;
			curr = next;
		}
		head->next = tail;
	}

	bool validate(NODE* pred, NODE* curr) {
		return (pred->removed == false)
			&& (curr->removed == false)
			&& (pred->next == curr);
	}

	bool add(int x)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;
			while (curr->value < x) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();

			if (true == validate(pred, curr)) {

				if (curr->value == x) {
					pred->unlock();
					curr->unlock();
					return false;
				}
				else
				{
					auto n = get_new_node(x);
					n->next = curr;
					pred->next = n;
					pred->unlock();
					curr->unlock();
					return true;
				}
			}

			pred->unlock();
			curr->unlock();
		}
	}

	bool remove(int x)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;
			while (curr->value < x) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();
			if (false == validate(pred, curr)) {
				pred->unlock(); curr->unlock();
				continue;
			}

			if (curr->value == x) {
				curr->removed = true;
				pred->next = curr->next;
				pred->unlock();
				curr->unlock();
				delete_node(curr);
				return true;
			}
			else
			{
				pred->unlock();
				curr->unlock();
				return false;
			}
		}
	}
	bool contains(int x)
	{
		while (true) {
			NODE* curr = head->next;
			while (curr->value < x)
				curr = curr->next;
			return (curr->value == x) && (curr->removed == false);
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
	LF_NODE* get_ptr_and_mark(bool *mark) {
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
	MR next;
	volatile int epoch;
	LF_NODE(int v) : value(v) {}
};

class LF_SET {
	LF_NODE* head, * tail;
public:
	LF_SET() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = new LF_NODE(-1);
		tail = new LF_NODE(1000000001);
		head->next = tail;
	}

	~LF_SET() {
		clear();
		delete head;
		delete tail;
	}

	void clear() {
		LF_NODE* curr = head->next.get_ptr();
		while (curr != tail) {
			LF_NODE* next = curr->next.get_ptr();
			delete curr;
			curr = next;
		}
		head->next = tail;
	}

	void find(LF_NODE*& pred, LF_NODE*& curr, int x)
	{
		while (true) {
		retry:
			pred = head;
			curr = pred->next.get_ptr();
			while (true) {
				bool marked = false;
				LF_NODE* succ = curr->next.get_ptr_and_mark(&marked);
				while (marked) {
					if (false == pred->next.CAS(curr, succ, false, false))
						goto retry;
					curr = succ;
					succ = curr->next.get_ptr_and_mark(&marked);
				}
				if (curr->value >= x)
					return;
				pred = curr;
				curr = succ;
			}
		}
	}

	bool add(int x)
	{
		while (true) {
			LF_NODE* pred, * curr;
			find(pred, curr, x);
			if (curr->value == x) {
				return false;
			}
			else {
				auto n = new LF_NODE(x);
				n->next = curr;
				if (true == pred->next.CAS(curr, n, false, false))
					return true;
				else
					delete n;
			}
		}
	}

	bool remove(int x)
	{
		while (true) {
			LF_NODE* pred, * curr;
			find(pred, curr, x);

			if (curr->value == x) {
				LF_NODE* succ = curr->next.get_ptr();
				if (false == curr->next.CAS(succ, succ, false, true))
					continue;
				pred->next.CAS(curr, succ, false, false);
				return true;
			}
			else {
				return false;
			}
		}
	}
	bool contains(int x)
	{
		while (true) {
			LF_NODE* curr = head->next.get_ptr();
			while (curr->value < x)
				curr = curr->next.get_ptr();
			return (curr->value == x) && (curr->next.get_mark() == false);
		}
	}

	void print20()
	{
		LF_NODE* curr = head->next.get_ptr();
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next.get_ptr())
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};

volatile int num_threads = 0;
thread_local int thread_id = 0;

class EBR {
	std::queue<LF_NODE*> node_pool[MAX_THREADS];
	std::atomic<int> epoch;
	struct EPOCH {
		alignas (64) std::atomic<int> thread_epoch;
	};
	EPOCH thread_epochs[MAX_THREADS];
public:
	EBR() : epoch(0) {
		for (int i = 0; i < MAX_THREADS; ++i)
			thread_epochs[i].thread_epoch = std::numeric_limits<int>::max();
	}
	~EBR() {
		recycle_all();
	}
	void recycle_all() {
		for (int i = 0; i < MAX_THREADS; ++i) {
			while (false == node_pool[i].empty()) {
				auto n = node_pool[i].front();
				node_pool[i].pop();
				delete n;
			}
		}
	}
	LF_NODE* getnode(int v) {
		if (false == node_pool[thread_id].empty()) {
			auto n = node_pool[thread_id].front();
			int epoch = n->epoch;
			bool can_use = true;
			for (int i = 0; i < num_threads; ++i) {
				int te = thread_epochs[i].thread_epoch;
				if (te <= epoch) {
					can_use = false;
					break;
				}
			}

			if (true == can_use) {
				node_pool[thread_id].pop();
				n->value = v;
				n->next.set_ptr(nullptr);
				return n;
			}
		}
		return new LF_NODE(v);
	}
	void recycle(LF_NODE* n) {
		n->epoch = epoch;
		node_pool[thread_id].push(n);
	}
	void enter() {
		thread_epochs[thread_id].thread_epoch = ++epoch;
	}
	void leave() {
		thread_epochs[thread_id].thread_epoch = std::numeric_limits<int>::max();
	}
};

class LF_SET_EBR {
	EBR ebr;
	LF_NODE* head, * tail;
public:
	LF_SET_EBR() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = new LF_NODE(-1);
		tail = new LF_NODE(1000000001);
		head->next = tail;
	}

	~LF_SET_EBR() {
		clear();
		delete head;
		delete tail;
	}

	void clear() {
		LF_NODE* curr = head->next.get_ptr();
		while (curr != tail) {
			LF_NODE* next = curr->next.get_ptr();
			delete curr;
			curr = next;
		}
		head->next = tail;
	}

	void find(LF_NODE*& pred, LF_NODE*& curr, int x)
	{
		while (true) {
		retry:
			pred = head;
			curr = pred->next.get_ptr();
			while (true) {
				bool marked = false;
				LF_NODE* succ = curr->next.get_ptr_and_mark(&marked);
				while (marked) {
					if (false == pred->next.CAS(curr, succ, false, false))
						goto retry;
					ebr.recycle(curr);
					curr = succ;
					succ = curr->next.get_ptr_and_mark(&marked);
				}
				if (curr->value >= x)
					return;
				pred = curr;
				curr = succ;
			}
		}
	}

	bool add(int x)
	{
		ebr.enter();
		while (true) {
			LF_NODE* pred, * curr;
			find(pred, curr, x);
			if (curr->value == x) {
				ebr.leave();
				return false;
			}
			else {
				auto n = ebr.getnode(x);
				n->next = curr;
				if (true == pred->next.CAS(curr, n, false, false)) {
					ebr.leave();
					return true;
				} else
					delete n;
			}
		}
	}

	bool remove(int x)
	{
		ebr.enter();
		while (true) {
			LF_NODE* pred, * curr;
			find(pred, curr, x);

			if (curr->value == x) {
				LF_NODE* succ = curr->next.get_ptr();
				if (false == curr->next.CAS(succ, succ, false, true))
					continue;
				if (true == pred->next.CAS(curr, succ, false, false))
					ebr.recycle(curr);
				ebr.leave();
				return true;
			}
			else {
				ebr.leave();
				return false;
			}
		}
	}
	bool contains(int x)
	{
		ebr.enter();
		while (true) {
			LF_NODE* curr = head->next.get_ptr();
			while (curr->value < x)
				curr = curr->next.get_ptr();
			auto result = (curr->value == x) && (curr->next.get_mark() == false);
			ebr.leave();
			return result;
		}
	}

	void print20()
	{
		LF_NODE* curr = head->next.get_ptr();
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next.get_ptr())
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};

class STD_SET {
	std::set<int> m_set;
	std::mutex set_lock;
public:
	STD_SET() {	}
	~STD_SET() { }

	void clear() {
		m_set.clear();
	}

	bool add(int x)
	{
		//std::lock_guard <std::mutex> lg(set_lock);
		if (0 == m_set.count(x)) {
			m_set.insert(x);
			return true;
		}
		else
		{
			return false;
		}
	}

	bool remove(int x)
	{
		//std::lock_guard <std::mutex> lg(set_lock);
		if (0 == m_set.count(x)) {
			return false;
		}
		else
		{
			m_set.erase(x);
			return true;
		}
	}

	bool contains(int x)
	{
		//std::lock_guard <std::mutex> lg(set_lock);
		return 0 != m_set.count(x);
	}

	void print20()
	{
		int count = 20;
		for (auto v : m_set) {
			std::cout << v << ", ";
			if (--count == 0) break;
		}
		std::cout << "\n";
	}
};

enum INVOCATION_TYPE {
	ADD,
	REMOVE,
	CONTAINS
};

class INVOCATION {
public:
	INVOCATION_TYPE type;
	int key;
	INVOCATION(INVOCATION_TYPE t, int k) : type(t), key(k) {}
	INVOCATION() {}
};

class RESPONSE {
public:
	bool result;
	RESPONSE(bool r) : result(r) {}
};

class SEQ_STD_SET {
	std::set<int> m_set;
public:
	RESPONSE apply(INVOCATION inv) {
		switch (inv.type) {
		case ADD:
			if (0 == m_set.count(inv.key)) {
				m_set.insert(inv.key);
				return RESPONSE(true);
			}
			else
			{
				return RESPONSE(false);
			}
		case REMOVE:
			if (0 == m_set.count(inv.key)) {
				return RESPONSE(false);
			}
			else
			{
				m_set.erase(inv.key);
				return RESPONSE(true);
			}
		case CONTAINS:
			return RESPONSE(0 != m_set.count(inv.key));
		default:
			std::cout << "Error\n";
			exit(-1);
		}
	}
	void print20()
	{
		int count = 20;
		for (auto v : m_set) {
			std::cout << v << ", ";
			if (--count == 0) break;
		}
		std::cout << "\n";
	}
};

class UNODE;
class CON_NEXT {
	UNODE* next{ nullptr };
	void CAS(UNODE** addr, UNODE* expected, UNODE* desired) {
		std::atomic_compare_exchange_strong(
			reinterpret_cast<std::atomic<UNODE*>*>(addr),
			&expected, desired);
	}
public:
	UNODE* decide(UNODE* v) {
		CAS(&next, nullptr, v);
		return next;
	}
	void clear() {
		next = nullptr;
	}
};

class UNODE {
public:
	INVOCATION inv;
	int seq;
	UNODE* next;
	CON_NEXT decide_next;
	UNODE(INVOCATION i, int s) : inv(i), seq(s), next(nullptr) {}
	UNODE() : seq(0), next(nullptr) {}
	void clear() {
		decide_next.clear();
		next = nullptr;
	}
};

class LFU_STD_SET {
	UNODE tail;
	UNODE* head[MAX_THREADS];
	UNODE* max_head() {
		UNODE* max = head[0];
		for (int i = 1; i < num_threads; ++i) {
			if (max->seq < head[i]->seq)
				max = head[i];
		}
		return max;
	}
public:
	LFU_STD_SET() {
		for (int i = 0; i < MAX_THREADS; ++i)
			head[i] = &tail;
	}
	RESPONSE apply(INVOCATION inv) {
		int th_id = thread_id;
		UNODE* prefer = new UNODE(inv, 0);
		while (prefer->seq == 0) {
			UNODE* before = max_head();
			UNODE *after = before->decide_next.decide(prefer);
			before->next = after;
			after->seq = before->seq + 1;
			head[th_id] = after;
		}

		SEQ_STD_SET seq_set;
		UNODE* curr = tail.next;
		while (curr != prefer) {
			seq_set.apply(curr->inv);
			curr = curr->next;
		}
		return seq_set.apply(inv);
	}
	void clear()
	{
		tail.clear();
		for (int i = 0; i < MAX_THREADS; ++i)
			head[i] = &tail;
	}
	void print20()
	{
		SEQ_STD_SET seq_set;
		UNODE* curr = tail.next;
		while (curr != nullptr) {
			seq_set.apply(curr->inv);
			curr = curr->next;
		}
		seq_set.print20();
	}
};

class LFU_SET {
	LFU_STD_SET lfu_set;
public:
	LFU_SET() {}
	~LFU_SET() {}

	void clear() {
		lfu_set.clear();
	}

	bool add(int x)
	{
		RESPONSE res = lfu_set.apply(INVOCATION(ADD, x));
		return res.result;
	}

	bool remove(int x)
	{
		RESPONSE res = lfu_set.apply(INVOCATION(REMOVE, x));
		return res.result;
	}

	bool contains(int x)
	{
		RESPONSE res = lfu_set.apply(INVOCATION(CONTAINS, x));
		return res.result;
	}

	void print20()
	{
		lfu_set.print20();
	}
};

constexpr int MAX_LEVEL = 9;
class SKNODE {
public:
	int value;
	SKNODE* volatile next[MAX_LEVEL + 1];
	int top_level;
	std::recursive_mutex node_lock;
	volatile bool removed = false;
	volatile bool fully_linked = false;
	SKNODE(int v, int top) : value(v), top_level(top), removed(false), fully_linked(false) {
		for (auto& p : next) p = nullptr;
	}
	SKNODE() : value(-1), top_level(0), removed (false), fully_linked(false) {
		for (auto& p : next) p = nullptr;
	}
};

class C_SKLIST {
	SKNODE* head, * tail;
	std::mutex set_lock;
public:
	C_SKLIST() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = new SKNODE(-1, MAX_LEVEL);
		tail = new SKNODE(1000000001, MAX_LEVEL);
		for (auto& p : head->next) p = tail;
	}

	~C_SKLIST() {
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

	void find(SKNODE* preds[], SKNODE* succs[], int x)
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
		SKNODE* preds[MAX_LEVEL + 1];
		SKNODE *currs[MAX_LEVEL + 1];
		set_lock.lock();
		find(preds, currs, x);

		if (currs[0]->value == x) {
			set_lock.unlock();
			return false;
		}
		else
		{
			int level = 0;
			for (level = 0; level < MAX_LEVEL; level++) {
				if (rand() % 2 == 0)break;
			}
			auto n = new SKNODE(x, level);
			for (int i = 0; i <= level; i++) {
				n->next[i] = currs[i];
				preds[i]->next[i] = n;
			}
			set_lock.unlock();
			return true;
		}
	}

	bool remove(int x)
	{
		SKNODE* preds[MAX_LEVEL + 1];
		SKNODE* currs[MAX_LEVEL + 1];
		
		set_lock.lock();
		find(preds, currs, x);

		if (currs[0]->value == x) {
			for (int i = 0; i <= currs[0]->top_level; i++) {
				preds[i]->next[i] = currs[0]->next[i];
			}
			set_lock.unlock();
			delete currs[0];
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
		SKNODE* preds[MAX_LEVEL + 1];
		SKNODE* currs[MAX_LEVEL + 1];
		set_lock.lock();
		find(preds, currs, x);
		if (currs[0]->value == x) {
			set_lock.unlock();
			return true;
		}
		else
		{
			set_lock.unlock();
			return false;
		}
	}

	void print20()
	{
		SKNODE* curr = head->next[0];
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next[0])
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};

class LFSKNODE;
class SKMR {
	volatile long long ptr_and_mark;
public:
	SKMR(LFSKNODE* p)
	{
		ptr_and_mark = reinterpret_cast<long long>(p);
	}
	SKMR() : ptr_and_mark(0) {}
	LFSKNODE* get_ptr()
	{
		long long temp = ptr_and_mark & 0xFFFFFFFFFFFFFFFE;
		return reinterpret_cast<LFSKNODE*>(temp);
	}
	bool get_mark() { return (ptr_and_mark & 1) == 1; }
	void set_ptr(LFSKNODE* p) {
		ptr_and_mark = reinterpret_cast<long long>(p);
	}
	LFSKNODE* get_ptr_and_mark(bool* mark) {
		long long temp = ptr_and_mark;
		*mark = (temp & 1) == 1;
		temp = temp & 0xFFFFFFFFFFFFFFFE;
		return reinterpret_cast<LFSKNODE*>(temp);
	}
	bool CAS(LFSKNODE* old_ptr, LFSKNODE* new_ptr,
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

class LFSKNODE {
public:
	int value;
	SKMR next[MAX_LEVEL + 1];
	volatile int top_level;
	LFSKNODE(int v, int top) : value(v), top_level(top) {
		for (auto& p : next) p = nullptr;
	}
	LFSKNODE() : value(-1), top_level(0) {
		for (auto& p : next) p = nullptr;
	}
};

class LF_SKLIST {
	LFSKNODE* head, * tail;
public:
	LF_SKLIST() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = new LFSKNODE(-1, MAX_LEVEL);
		tail = new LFSKNODE(1000000001, MAX_LEVEL);
		for (auto& p : head->next) p = tail;
	}

	~LF_SKLIST() {
		clear();
		delete head;
		delete tail;
	}

	void clear() {
		LFSKNODE* curr = head->next->get_ptr();
		while (curr != tail) {
			LFSKNODE* next = curr->next[0].get_ptr();
			delete curr;
			curr = next;
		}
		for (auto& p : head->next) p = tail;
	}

	bool find(LFSKNODE* preds[], LFSKNODE* currs[], int x)
	{
		retry :
		preds[MAX_LEVEL] = head;
		for (int level = MAX_LEVEL; level >= 0; level--) {
			if (MAX_LEVEL != level)
				preds[level] = preds[level + 1];
			currs[level] = preds[level]->next[level].get_ptr();
			while (true) {
				bool removed = false;
				LFSKNODE* succ = currs[level]->next[level].get_ptr_and_mark(&removed);
				while (true == removed) {
					if (false == preds[level]->next[level].CAS(currs[level], succ, false, false))
						goto retry;
					currs[level] = succ;
					succ = currs[level]->next[level].get_ptr_and_mark(&removed);
				}
				if (currs[level]->value < x) {
					preds[level] = currs[level];
					currs[level] = succ;
				}
				else break;
			}
		}
		return currs[0]->value == x;
	}

	bool add(int x)
	{
		int top_level = rand() % MAX_LEVEL;
		int bottom_level = 0;
		LFSKNODE* preds[MAX_LEVEL + 1];
		LFSKNODE* currs[MAX_LEVEL + 1];

		while (true) {
			bool found = find(preds, currs, x);
			if (found) return false;
			LFSKNODE* n = new LFSKNODE(x, top_level);
			for(int level = bottom_level; level <= top_level; level++) {
				LFSKNODE* succ = currs[level];
				n->next[level].set_ptr(succ);
			}

			LFSKNODE* pred = preds[bottom_level];
			LFSKNODE* succ = currs[bottom_level];
			n->next[bottom_level].set_ptr(succ);
			if(!pred->next[bottom_level].CAS(succ, n, false, false)) {
				delete n;
				continue;
			}

			for(int level = bottom_level + 1; level <= top_level; level++) {
				while (true) {
					pred = preds[level];
					succ = currs[level];
					if (pred->next[level].CAS(succ, n, false, false))
						break;
					find(preds, currs, x);
				}
			}

			return true;
		}
	}

	bool remove(int x)
	{
		LFSKNODE* preds[MAX_LEVEL + 1];
		LFSKNODE* currs[MAX_LEVEL + 1];

		bool found = find(preds, currs, x);
		if (false == found) return false;

		LFSKNODE* victim = currs[0];

		for (int level = victim->top_level; level >= 1; level--) {
			bool marked = false;
			LFSKNODE* succ = victim->next[level].get_ptr_and_mark(&marked);
			while (false == marked) {
				victim->next[level].CAS(succ, succ, false, true);
				succ = victim->next[level].get_ptr_and_mark(&marked);
			}
		}
		bool marked = false;
		LFSKNODE* succ = victim->next[0].get_ptr_and_mark(&marked);
		while (true) {
			bool i_marked_it = victim->next[0].CAS(succ, succ, false, true);
			succ = victim->next[0].get_ptr_and_mark(&marked);
			if (i_marked_it) {
				find(preds, currs, x);
				return true;
			}
			else if (marked) {
				return false;
			}
		}
	}
	bool contains(int x)
	{
		LFSKNODE* pred;
		LFSKNODE* curr;
		LFSKNODE* succ = nullptr;
		pred = head;
		for (int i = MAX_LEVEL; i >= 0; i--) {
			curr = pred->next[i].get_ptr();
			bool removed = false;
			while (1)
			{
				succ = curr->next[i].get_ptr_and_mark(&removed);
				while (removed) {
					curr = succ;
					succ = curr->next[i].get_ptr_and_mark(&removed);
				}
				if (curr->value < x) {
					pred = curr;
					curr = succ;
				}
				else break;
			}
		}
		return curr->value == x;
	}

	void print20()
	{
		LFSKNODE* curr = head->next[0].get_ptr();
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next[0].get_ptr())
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};

class Z_SKLIST {
	SKNODE* head, * tail;
public:
	Z_SKLIST() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = new SKNODE(-1, MAX_LEVEL);
		tail = new SKNODE(1000000001, MAX_LEVEL);
		head->fully_linked = tail->fully_linked = true;
		for (auto& p : head->next) p = tail;
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

		while (true) {
			int found_level = find(preds, currs, x);
			if (found_level != -1) {
				SKNODE* node_found = currs[found_level];
				if (node_found->removed == true) continue;
				else {
					while (node_found->fully_linked == false);
					return false;
				}
			}

			int top_level = 0;
			for (int top_level = 0; top_level < MAX_LEVEL; top_level++)
				if (rand() % 2 == 0) break;
			bool valid = true;
			int highest_locked = -1;
			for (int i = 0; i <= top_level; i++) {
				preds[i]->node_lock.lock();
				highest_locked = i;
				valid = (preds[i]->removed == false)
					&& (currs[i]->removed == false)
					&& (currs[i] == preds[i]->next[i]);
				if (false == valid) break;
			}
			if (false == valid) {
				for (int i = 0; i <= highest_locked; i++)
					preds[i]->node_lock.unlock();
				continue;
			}
			SKNODE* n = new SKNODE(x, top_level);
			for (int i = 0; i <= top_level; i++)
				n->next[i] = currs[i];
			for (int i = 0; i <= top_level; i++)
				preds[i]->next[i] = n;
			n->fully_linked = true;
			for (int i = top_level; i >= 0; --i)
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
			int highest_locked = -1;
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
				find(preds, currs, x);
				continue;
			}
			for (int i = top_level; i >= 0; i--)
				preds[i]->next[i] = victim->next[i];
			for (int i = top_level; i >= 0; i--)
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

LF_SKLIST clist;

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
		for (int i = 1; i <= MAX_THREADS; i = i * 2) {
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
			recycle_nodes();
			std::cout << i << " Threads : SET = ";
			clist.print20();
			std::cout << "Exec time = " << exec_ms << "ms.  ";
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
		recycle_nodes();
		std::cout << "Thread Count = " << num_threads << ", Exec Time = " << time_span << "ms.\n";
		std::cout << "Result : ";  clist.print20();
	}
}
