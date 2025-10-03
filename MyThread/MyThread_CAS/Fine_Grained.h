#pragma once

#include "Node_raw.h"


typedef class F_SET
{
private:
	Node* head, * tail;
	std::mutex set_lock;
public:
	F_SET()
	{
		// Set의 범위는 [0, 1000]으로 제한하겠음
		head = new Node(INT_MIN);
		tail = new Node(INT_MAX);
		head->next = tail;
	}

	~F_SET()
	{
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		Node* curr = head->next;
		while (curr != tail)
		{
			Node* temp = curr;
			curr = curr->next;
			delete temp;
		}
		head->next = tail;
	}

	bool add(int x) {
		Node* pred = head;
		pred->lock();
		Node* curr = pred->next;
		curr->lock();

		// pred->value < x <= curr->value
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
		else {

			Node* n = new Node(x);
			n->next = curr;
			pred->next = n;

			curr->unlock();
			pred->unlock();
			return true;
		}

		return false;
	}

	bool remove(int x) {
		Node* pred = head;
		pred->lock();
		Node* curr = pred->next;
		curr->lock();

		while (curr->value < x) {
			pred->unlock();
			pred = curr;
			curr = curr->next;

			curr->lock();
		}

		if (curr->value != x) {
			pred->unlock();
			curr->unlock();
			return false;
		}
		else {
			pred->next = curr->next;

			pred->unlock();
			curr->unlock();

			delete curr;
			return true;
		}

		return false;
	}

	bool contains(int x) {
		Node* pred = head;
		pred->lock();
		Node* curr = pred->next;
		curr->lock();

		while (curr->value < x) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		bool found = (curr->value == x);

		pred->unlock();
		curr->unlock();
		return found;
	}
	void print20()
	{
		Node* curr = head->next;
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next)
			std::cout << curr->value << " ";
		std::cout << std::endl;
	}
}SET;
