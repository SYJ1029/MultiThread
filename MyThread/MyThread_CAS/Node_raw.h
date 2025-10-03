#pragma once

#include "pch.h"


class Node
{
public:
	int value;
	Node* next;
	bool removed = false; // ���� ������ ���� ǥ��
	Node(int val) : value(val), next(nullptr) {}

	void lock() { nodeLock.lock(); }
	void unlock() { nodeLock.unlock(); }

private:
	std::mutex nodeLock;
}; 
