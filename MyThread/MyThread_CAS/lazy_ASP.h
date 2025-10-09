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
		// Set�� ������ [0, 1000]���� �����ϰ���
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
			std::shared_ptr<Node> pred(head);
			std::shared_ptr<Node> curr = pred -> next;

			while (curr->value < x) {
				pred = (curr);
				curr = curr->next;
			}

			// ���⼭ pred�� curr�� lock�� �Ǵ�
			pred->lock();
			curr->lock();
			if (validate(pred, curr)) {
				std::atomic<std::shared_ptr<Node>> n = std::make_shared<Node>(x);

				if (curr->value == x) {
					curr->unlock();
					pred->unlock();
					return false;
				}

				n.load()->next = (curr);
				pred->next.store(n);

				curr->unlock();
				pred->unlock();
				return true;
			}


			// validate ���� ��, ��� ���� �� �ٽ� �õ�
			pred->unlock();
			curr->unlock();

		}

		// while���� �������� ���⿡ �����ϴ� break���� �����Ƿ� �����ؼ��� �ȵǴ� �ڵ�
		// �̰��� �����ߴٸ� �����ΰ� �߸��� ��
		return false;
	}

	bool remove(int x) {

		while (true) {
			std::shared_ptr<Node> pred(head);
			std::shared_ptr<Node> curr = pred->next;

			while (curr->value < x) {
				pred = (curr);
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

				curr->marked = true; // ���� ����

				pred->next.store(curr->next);
				curr->unlock();
				pred->unlock();
				return true;
			}

			pred->unlock();
			curr->unlock();
		}
		return false;
	}

	bool contains(int x) {
		while (true) {
			std::shared_ptr<Node> pred(head);
			std::shared_ptr<Node> curr = pred->next;

			while (curr->value < x) {
				pred = (curr);
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

	bool validate(const std::shared_ptr<Node>& pred, const std::shared_ptr<Node>& curr) {
		return !pred->marked && !curr->marked && pred->next.load() == curr;
	}

	void print20()
	{
		ASPOINTER curr; curr.store(head.load()->next);
		for (int i = 0; i < 20 && curr.load() != tail.load(); i++, curr.exchange(curr.load()->next))
			std::cout << curr.load()->value << " ";
		std::cout << std::endl;
	}
}SET;
