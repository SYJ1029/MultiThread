#pragma once

#include "Node_AS.h"
//#include <vector>

#define  ASPOINTER std::atomic<std::shared_ptr<Node>>

typedef class L_SET
{
private:
	std::atomic<std::shared_ptr<Node>> head, tail;
public:
	L_SET()
	{
		// Set의 범위는 [0, 1000]으로 제한하겠음
		head = std::make_shared<Node>(INT_MIN);
		tail = std::make_shared<Node>(INT_MAX);

		head.load()->next.store(tail);
	}

	~L_SET()
	{
		clear();
	}

	void clear()
	{
		head.load()->next.store(tail);
	}

	bool add(int x) {


		while (true) {
			std::atomic<std::shared_ptr<Node>> pred;
			pred.store(head);
			std::atomic<std::shared_ptr<Node>> curr;
			curr.store(pred.load()->next);

			while (curr.load()->value < x) {
				pred.store(curr);
				curr.store(curr.load()->next);
			}

			// 여기서 pred와 curr에 lock을 건다
			pred.load()->lock();
			curr.load()->lock();
			if (validate(pred, curr)) {
				std::atomic<std::shared_ptr<Node>> n = std::make_shared<Node>(x);

				if (curr.load()->value == x) {
					curr.load()->unlock();
					pred.load()->unlock();
					return false;
				}

				n.load()->next.store(curr);
				pred.load()->next.store(n);

				curr.load()->unlock();
				pred.load()->unlock();
				return true;
			}


			// validate 실패 시, 잠금 해제 후 다시 시도
			pred.load()->unlock();
			curr.load()->unlock();

		}

		// while문을 빠져나와 여기에 도달하는 break문은 없으므로 도달해서는 안되는 코드
		// 이곳에 도달했다면 무엇인가 잘못된 것
		return false;
	}

	bool remove(int x) {

		while (true) {
			std::atomic<std::shared_ptr<Node>> pred;
			pred.store(head);
			std::atomic<std::shared_ptr<Node>> curr;
			curr.store(pred.load()->next);

			while (curr.load()->value < x) {
				pred.store(curr);
				curr.store(curr.load()->next);
			}
			pred.load()->lock();
			curr.load()->lock();

			if (validate(pred, curr)) {
				if (curr.load()->value != x) {
					curr.load()->unlock();
					pred.load()->unlock();
					return false;
				}

				curr.load()->marked = true; // 논리적 삭제

				pred.load()->next.store(curr.load()->next);
				curr.load()->unlock();
				pred.load()->unlock();
				return true;
			}

			pred.load()->unlock();
			curr.load()->unlock();
		}
		return false;
	}

	bool contains(int x) {
		while (true) {
			std::atomic<std::shared_ptr<Node>> pred; pred.store(head);
			std::atomic<std::shared_ptr<Node>> curr; curr.store(pred.load()->next);

			while (curr.load()->value < x) {
				pred.store(curr);
				curr.store(curr.load()->next);
			}

			pred.load()->lock();
			curr.load()->lock();
			if (validate(pred, curr)) {

				bool found = (curr.load()->value == x);
				curr.load()->unlock();
				pred.load()->unlock();
				return found;
			}

			pred.load()->unlock();
			curr.load()->unlock();
		}

		return false;
	}

	bool validate(const ASPOINTER& pred, const ASPOINTER& curr) {
		return !pred.load()->marked && !curr.load()->marked && pred.load()->next.load() == curr.load();
	}

	void print20()
	{
		ASPOINTER curr; curr.store(head.load()->next);
		for (int i = 0; i < 20 && curr.load() != tail.load(); i++, curr.exchange(curr.load()->next))
			std::cout << curr.load()->value << " ";
		std::cout << std::endl;
	}
}SET;
