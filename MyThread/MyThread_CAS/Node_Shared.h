#pragma once

#include "pch.h"

class Node
{
public:
	int value;
	std::shared_ptr<Node> next;
	bool marked = false; // ���� ������ ���� ǥ��
	Node(int val) : value(val), next(nullptr) {}

	void lock() { nodeLock.lock(); }
	void unlock() { nodeLock.unlock(); }

private:
	std::mutex nodeLock;
};