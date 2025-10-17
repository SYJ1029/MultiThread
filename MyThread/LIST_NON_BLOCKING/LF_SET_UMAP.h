#pragma once

#include "pch.h"
#include "LF_NODE_EPOCH.h"
#include <queue>

volatile int num_threads = 0;

// thread_local 대신에 unordered_map + this_thread::get_id() 조합을 사용
#include <unordered_map>

std::unordered_map<std::thread::id , int> threadIdMap;

class EBR {
public:
	std::atomic<int> epoch;
	std::queue<LF_NODE*> node_pool[MAX_THREADS];
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
		// map으로부터 thread_id를 얻어온다.
		int thread_id = threadIdMap[std::this_thread::get_id()];

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
		// map으로부터 thread_id를 얻어온다.
		int thread_id = threadIdMap[std::this_thread::get_id()];

		n->epoch = epoch;
		node_pool[thread_id].push(n);


	}
};

typedef class LF_SET_UMAP {
	EBR ebr;
	LF_NODE* head, * tail;
public:
	LF_SET_UMAP() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = new LF_NODE(std::numeric_limits<int>::min());
		tail = new LF_NODE(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~LF_SET_UMAP() {
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
				if (curr->value >= x) {
					return;
				}
				pred = curr;
				curr = succ;
			}
		}
	}

	bool add(int x)
	{
		// map으로부터 thread_id를 얻어온다.
		int thread_id = threadIdMap[std::this_thread::get_id()];

		//Atomic global EPOCH Counter를 증가시킨다.
		ebr.epoch++;
		ebr.thread_epochs[thread_id].thread_epoch.store(ebr.epoch);


		while (true) {
			LF_NODE* pred, * curr;
			find(pred, curr, x);
			if (curr->value == x) {
				ebr.thread_epochs[thread_id].thread_epoch.store(std::numeric_limits<int>::max());
				return false;
			}
			else {
				auto n = ebr.getnode(x);
				n->next = curr;
				if (true == pred->next.CAS(curr, n, false, false)) {
					ebr.thread_epochs[thread_id].thread_epoch.store(std::numeric_limits<int>::max());
					return true;
				}
				else
					delete n;
			}
		}
	}

	bool remove(int x)
	{
		// map으로부터 thread_id를 얻어온다.
		int thread_id = threadIdMap[std::this_thread::get_id()];

		ebr.epoch++;
		ebr.thread_epochs[thread_id].thread_epoch.store(ebr.epoch);

		while (true) {
			LF_NODE* pred, * curr;
			find(pred, curr, x);

			if (curr->value == x) {
				LF_NODE* succ = curr->next.get_ptr();
				if (false == curr->next.CAS(succ, succ, false, true))
					continue;
				if (true == pred->next.CAS(curr, succ, false, false))
					ebr.recycle(curr);

				return true;
			}
			else {
				ebr.thread_epochs[thread_id].thread_epoch.store(std::numeric_limits<int>::max());
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
}SET;

// thread id 설정
void SetId(int id)
{
	threadIdMap[std::this_thread::get_id()] = id;
}

// thread id 초기화
void ClearId()
{
	threadIdMap.clear();
}