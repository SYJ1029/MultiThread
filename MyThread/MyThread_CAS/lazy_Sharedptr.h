#pragma once

#include "Node_Shared.h"

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
		std::shared_ptr<Node> curr = head->next;
		while (curr != tail)
		{
			std::shared_ptr<Node> temp = curr;
			curr = temp->next;
			temp.reset();
		}
		head->next = tail;
	}

	bool add(int x) {


		while (true) {
			std::shared_ptr<Node> pred(head);
			auto curr = pred->next;

			while (curr->value < x) {
				pred = curr;
				curr = curr->next;
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
				pred->next = n;

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

				curr->marked = true; // ���� ����

				pred->next = curr->next;
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

	bool validate(std::shared_ptr<Node> pred, std::shared_ptr<Node> curr) {
		return !pred->marked && !curr->marked && pred->next == curr;
	}

	void print20()
	{
		std::shared_ptr<Node> curr(head->next);
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next)
			std::cout << curr->value << " ";
		std::cout << std::endl;
	}
}SET;
