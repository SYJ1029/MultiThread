#pragma once

#include "pch.h"

typedef class Node_ASP
{
public:
	int value;
	std::atomic<std::shared_ptr<Node_ASP>> next;
	bool marked = false; // 논리적 삭제를 위한 표시
	Node_ASP(int val) : value(val), next(nullptr) {}

	void lock() { nodeLock.lock(); }
	void unlock() { nodeLock.unlock(); }

private:
	std::mutex nodeLock;
}Node; 
