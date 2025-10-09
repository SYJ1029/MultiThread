#pragma once

#include "Node_Shared.h"
//#include <vector>

typedef class L_SET
{
private:
	std::shared_ptr<Node> head, tail;
public:
	L_SET()
	{
		// Set�� ������ [0, 1000]���� �����ϰ���
		head = std::make_shared<Node>(INT_MIN);
		tail = std::make_shared<Node>(INT_MAX);
		head->next = tail;
	}

	~L_SET()
	{
		clear();
	}

	void clear()
	{
		head->next = tail;
	}

	bool add(int x) {


		while (true) {
			std::shared_ptr<Node> pred(head);
			std::shared_ptr<Node> curr = std::atomic_load(&pred->next);

			while (curr->value < x) {
				pred = curr;
				curr = std::atomic_load(&curr->next);
			}

			// ���⼭ pred�� curr�� lock�� �Ǵ�
			pred->lock();
			curr->lock();
			if (validate(pred, curr)) {
				std::shared_ptr<Node> n = std::make_shared<Node>(x);

				if (curr->value == x) {
					curr->unlock();
					pred->unlock();
					return false;
				}

				n->next = curr;
				std::atomic_exchange(&pred->next, n);

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
			std::shared_ptr<Node> curr = std::atomic_load(&pred->next);

			while (curr->value < x) {
				pred = curr;
				curr = std::atomic_load(&curr->next);
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

				std::atomic_exchange(&pred->next, std::atomic_load(&curr->next));
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
			std::shared_ptr<Node> curr = std::atomic_load(&pred->next);

			while (curr->value < x) {
				pred = curr;
				curr = std::atomic_load(&curr->next);
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
		return !pred->marked && !curr->marked && std::atomic_load(&pred->next) == curr;
	}

	void print20()
	{
		std::shared_ptr<Node> curr(head->next);
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next)
			std::cout << curr->value << " ";
		std::cout << std::endl;
	}
}SET;
