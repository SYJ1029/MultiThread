#pragma once

#include "LF_NODE_EPOCH.h"

#include <queue>

volatile int num_threads = 0;
thread_local int thread_id = -1;	// 전역 변수이면서 모든 쓰레드가 다른 값을 가지게 한다.
									// thread_local 키워드가 붙어야 한다.

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

	void enter() {
		epoch++;
		thread_epochs[thread_id].thread_epoch.store(epoch);
	}

	void leave() {
		thread_epochs[thread_id].thread_epoch.store(std::numeric_limits<int>::max());
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
			// node_pool이 비어있지 않다면 하나를 꺼내서 재활용을 시도
			auto n = node_pool[thread_id].front();
			int epoch = n->epoch; // 꺼낸 노드의  epoch를 가지고 온다
			bool can_use = true;
			for (int i = 0; i < num_threads; ++i) {
				int te = thread_epochs[i].thread_epoch;	// 확인하는 스레드의 epoch를 가져온다
				if (te <= epoch) {
					// te < epoch라는 이야기는 스레드의 함수 중 누군가는 이 노드를 아직 참조하고 있을 수 있다는 뜻
					can_use = false;
					break;
				}
			}

			if (true == can_use) {
				// 모든 스레드의 epoch가 이 노드의 epoch보다 크므로 재활용 가능
				node_pool[thread_id].pop();
				n->value = v;
				n->next.set_ptr(nullptr);
				return n;
			}
		}
		return new LF_NODE(v);
	}
	void recycle(LF_NODE* n) {
		// 노드를 제거하지 않고 pool에 저장한다
		n->epoch = epoch;
		node_pool[thread_id].push(n);

		
	}
};

typedef class LF_SET_EBR {
	EBR ebr;
	LF_NODE* head, * tail;
public:
	LF_SET_EBR() {
		/* 값을 0부터 1000까지로 제한하겠음 */
		head = new LF_NODE(std::numeric_limits<int>::min());
		tail = new LF_NODE(std::numeric_limits<int>::max());
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
		//Atomic global EPOCH Counter를 증가시킨다.
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
				}
				else
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

			auto res = (curr->value == x) && (curr->next.get_mark() == false);

			ebr.leave();
			return res;
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


void SetId(int id)
{
	thread_id = id;
}


void ClearId() {} // EBR에서는 기능이 없다