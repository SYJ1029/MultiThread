#pragma once

#include "LF_NODE.h"

typedef class LF_SET {
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
			}
		}
	}

	bool remove(int x)
	{
		bool snip = false;
		while (true) {
			LF_NODE* pred, * curr;
			find(pred, curr, x);

			if (curr->value != x) {
				return false;
			}
			else
			{
				LF_NODE* succ = curr->next.get_ptr();
				snip = curr->next.CAS(succ, succ, false, true);
				if (!snip)
					continue;

				pred->next.CAS(curr, succ, false, false);

				return true;
			}
		}
	}
	bool contains(int x)
	{
		bool marked = false;
		LF_NODE* curr = head->next.get_ptr_and_mark(&marked);
		while (curr->value < x) {
			curr = curr->next.get_ptr_and_mark(&marked);
		}
		return (curr->value == x) && (!marked);

	}

	void print20()
	{
		LF_NODE* curr = head->next.get_ptr();
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next.get_ptr())
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
}SET;
