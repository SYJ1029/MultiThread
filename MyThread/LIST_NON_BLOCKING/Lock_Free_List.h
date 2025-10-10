#pragma once
#include "Node.h"

typedef class L_SET
{
private:
	Node* head, * tail;
public:
	L_SET();

	~L_SET();

	void clear();

	bool add(int x);

	bool remove(int x);

	bool contains(int x);

	bool validate(Node* pred, Node* curr);

	void print20();
}SET;
