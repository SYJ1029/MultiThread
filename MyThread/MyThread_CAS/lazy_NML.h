#pragma once

#include "Node_raw.h"
#include <queue>

std::queue<Node*> node_pool;
std::mutex pool_lock;

void recycle_nodes()
{
	while (false == node_pool.empty())
	{
		auto n = node_pool.front();
		node_pool.pop();
		delete n;
	}
}

Node* GetNewNode(int x) {
	return new Node(x);
}

void DeleteNode(Node* node) {
	std::lock_guard<std::mutex> lg(pool_lock);
	node_pool.push(node);
}

typedef class L_SET
{
private:
	Node* head, * tail;
public:
	L_SET()
	{
		// Set�� ������ [0, 1000]���� �����ϰ���
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

			// ���⼭ pred�� curr�� lock�� �Ǵ�
			pred->lock();
			curr->lock();
			if (validate(pred, curr)) {
				auto n = GetNewNode(x);

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

				curr->removed = true; // ���� ����

				pred->next = curr->next;
				curr->unlock();
				pred->unlock();
				DeleteNode(curr);
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

