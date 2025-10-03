#pragma once

#include "pch.h"

typedef class Node_SP
{
public:
	int value;
	std::shared_ptr<Node_SP> next;
	bool marked = false; // 논리적 삭제를 위한 표시
	Node_SP(int val) : value(val), next(nullptr) {}

	void lock() { nodeLock.lock(); }
	void unlock() { nodeLock.unlock(); }

private:
	std::mutex nodeLock;
}Node;