#pragma once

#include "pch.h"


class Node
{
public:
	int value;
	Node* next;
	bool removed = false; // 논리적 삭제를 위한 표시
	Node(int val) : value(val), next(nullptr) {}

	void lock() { nodeLock.lock(); }
	void unlock() { nodeLock.unlock(); }

private:
	std::mutex nodeLock;
}; 
