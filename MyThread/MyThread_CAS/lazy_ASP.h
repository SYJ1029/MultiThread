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
			std::atomic<std::shared_ptr<Node>> pred;
			pred.store(head);
			std::atomic<std::shared_ptr<Node>> curr;
			curr.store(pred.load()->next);

			while (curr.load()->value < x) {
				pred.store(curr);
				curr.store(curr.load()->next);
			}

			// ���⼭ pred�� curr�� lock�� �Ǵ�
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


			// validate ���� ��, ��� ���� �� �ٽ� �õ�
			pred.load()->unlock();
			curr.load()->unlock();

		}

		// while���� �������� ���⿡ �����ϴ� break���� �����Ƿ� �����ؼ��� �ȵǴ� �ڵ�
		// �̰��� �����ߴٸ� �����ΰ� �߸��� ��
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

				curr.load()->marked = true; // ���� ����

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
