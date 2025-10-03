#pragma once
#include "Node_raw.h"

typedef class O_SET
{
private:
	Node* head, * tail;
public:
	O_SET();
	~O_SET();

	void clear();

	bool add(int x);

	bool remove(int x);

	bool contains(int x);

	bool validate(Node* pred, Node* curr);

	void print20();
}SET;
