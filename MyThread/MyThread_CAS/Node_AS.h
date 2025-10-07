#pragma once

#include "pch.h"

typedef class Node_ASP
{
public:
	int value;
	std::atomic<std::shared_ptr<Node_ASP>> next;
	bool marked = false; // ���� ������ ���� ǥ��
	Node_ASP(int val) : value(val), next(nullptr) {}

	void lock() { nodeLock.lock(); }
	void unlock() { nodeLock.unlock(); }

private:
	std::mutex nodeLock;
}Node; 
