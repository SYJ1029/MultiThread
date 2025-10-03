#pragma once

#include "Node_raw.h"

typedef class L_SET
{
private:
	Node* head, * tail;
public:
	L_SET()
	{
		// Set의 범위는 [0, 1000]으로 제한하겠음
		head = new Node(INT_MIN);
		tail = new Node(INT_MAX);
		head->next = tail;
	}

	~L_SET()
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


		while (true) {
			Node* pred = head;
			Node* curr = pred->next;

			while (curr->value < x) {
				pred = curr;
				curr = curr->next;
			}

			// 여기서 pred와 curr에 lock을 건다
			pred->lock();
			curr->lock();
			if (validate(pred, curr)) {
				Node* n = new Node(x);

				if (curr->value == x) {
					curr->unlock();
					pred->unlock();
					return false;
				}

				n->next = curr;
				pred->next = n;

				curr->unlock();
				pred->unlock();
				return true;
			}


			// validate 실패 시, 잠금 해제 후 다시 시도
			pred->unlock();
			curr->unlock();

		}

		// while문을 빠져나와 여기에 도달하는 break문은 없으므로 도달해서는 안되는 코드
		// 이곳에 도달했다면 무엇인가 잘못된 것
		return false;
	}

	bool remove(int x) {

		while (true) {
			Node* pred = head;
			Node* curr = pred->next;

			while (curr->value < x) {
				pred = curr;
				curr = curr->next;
			}
			pred->lock();
			curr->lock();

			if (validate(pred, curr)) {
				if (curr->value != x) {
					curr->unlock();
					pred->unlock();
					return false;
				}

				curr->removed = true; // 논리적 삭제

				pred->next = curr->next;
				curr->unlock();
				pred->unlock();
				//delete curr;		// 낙천적 동기화에서는 실행하면 안되는 코드. 왜 일까?
				return true;
			}

			pred->unlock();
			curr->unlock();
		}
		return false;
	}

	bool contains(int x) {
		while (true) {
			Node* pred = head;
			Node* curr = pred->next;

			while (curr->value < x) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock();
			curr->lock();
			if (validate(pred, curr)) {

				bool found = (curr->value == x);
				curr->unlock();
				pred->unlock();
				return found;
			}

			pred->unlock();
			curr->unlock();
		}

		return false;
	}

	bool validate(Node* pred, Node* curr) {
		return !pred->removed && !curr->removed && pred->next == curr;
	}

	void print20()
	{
		Node* curr = head->next;
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next)
			std::cout << curr->value << " ";
		std::cout << std::endl;
	}
}SET;
