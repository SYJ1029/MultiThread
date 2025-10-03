#pragma once

#include "Node_raw.h"

typedef class C_SET
{
private:
	Node* head, * tail;
	std::mutex set_lock;
public:
	C_SET();
	~C_SET();


	void clear();

	bool add(int x);

	bool remove(int x);

	bool contains(int x);
	void print20();
}SET;
