#include "coarse_grained.h"

SET::C_SET()
{
	{
		// Set의 범위는 [0, 1000]으로 제한하겠음
		head = new Node(INT_MIN);
		tail = new Node(INT_MAX);
		head->next = tail;
	}
}

SET::~C_SET()
{

	clear();
	delete head;
	delete tail;

}
void SET::clear()
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


bool SET::add(int x)
{
	Node* pred = head;
	set_lock.lock();
	Node* curr = head->next;
	// pred < x <= curr
	while (curr->value < x) {
		pred = curr;
		curr = curr->next;
	}
	if (curr->value == x)
	{
		set_lock.unlock();
		return false;
	}
	else
	{
		auto n = new Node(x);
		n->next = curr;
		pred->next = n;
	}
	set_lock.unlock();
	return true;
}

bool SET::remove(int x)
{
	Node* pred = head;
	set_lock.lock();
	Node* curr = head->next;
	// pred < x <= curr
	while (curr->value < x) {
		pred = curr;
		curr = curr->next;
	}
	if (curr->value == x)
	{
		pred->next = curr->next;
		delete curr;
	}
	else {
		set_lock.unlock();
		return false;
	}
	set_lock.unlock();
	return true;
}

bool SET::contains(int x)
{
	set_lock.lock();
	Node* curr = head->next;
	while (curr->value < x)
		curr = curr->next;
	if (curr->value != x) {
		set_lock.unlock();
		return false;
	}
	set_lock.unlock();
	return true;
}

void SET::print20()
{
	Node* curr = head->next;
	for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next)
		std::cout << curr->value << " ";
	std::cout << std::endl;
}

