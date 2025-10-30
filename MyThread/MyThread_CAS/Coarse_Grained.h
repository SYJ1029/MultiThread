#pragma once

#include "Node_raw.h"
#include <set>

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

class STD_SET {
	std::set<int> m_set;
	std::mutex set_lock;
public:
	STD_SET() {}
	~STD_SET() {}

	void clear() {
		m_set.clear();
	}
	
	bool add(int x)
	{
		//std::lock_guard<std::mutex> lg(set_lock);
		if(0 == m_set.count(x)) {
			m_set.insert(x);
			return true;
		}
		else 
		{
			return false;
		}
	}

	bool remove(int x)
	{
		//std::lock_guard<std::mutex> lg(set_lock);
		if (0 == m_set.count(x)) {
			return false;
		}
		else
		{
			m_set.erase(x);
			return true;
		}
	}

	bool contains(int x)
	{
		//std::lock_guard<std::mutex> lg(set_lock);
		return (0 != m_set.count(x));
	}

	void print20()
	{
		int count = 20;
		for (auto v : m_set)
		{
			std::cout << v << ", ";
			if (--count == 0) break;
		}
	}
};


enum INVOCATION_TYPE {
	ADD,
	REMOVE,
	CONTAINS
};

class INVOCATION {
public:
	INVOCATION_TYPE type;
	int value;
	INVOCATION(INVOCATION_TYPE t, int v) : type(t), value(v) {}
	INVOCATION() {}
};

class RESPONSE {
public:
	bool result;
	RESPONSE(bool r) : result(r) {}
};

class SEQ_STD_SET {
	std::set<int> m_set;
public:
	RESPONSE apply(INVOCATION inv) 
	{
		switch (inv.type) {
		case ADD:
			if (0 == m_set.count(inv.value)) {
				m_set.insert(inv.value);
				return RESPONSE(true);
			}
			else
			{
				return RESPONSE(false);
			}
		case REMOVE:
			if (0 == m_set.count(inv.value)) {
				return RESPONSE(false);
			}
			else
			{
				m_set.erase(inv.value);
				return RESPONSE(true);
			}
		case CONTAINS:
			return RESPONSE(0 != m_set.count(inv.value));
		default:
			return RESPONSE(false);
		}
	}

	void print20()
	{
		int count = 20;
		for (auto v : m_set)
		{
			std::cout << v << ", ";
			if (--count == 0) break;
		}
	}
};


thread_local int thread_id{ 0 };

class UNODE;
class CON_NEXT {
	UNODE* next{ nullptr };
	void CAS(UNODE** addr, UNODE* expected, UNODE* desired) {
		std::atomic_compare_exchange_strong(
			reinterpret_cast<std::atomic<UNODE*>*>(addr),
			&expected,
			desired);
	}
public:
	UNODE* decide(UNODE* v){
		CAS(&next, nullptr, v);
		return next;
	}

	void clear() {
		next = nullptr;
	}
};

class UNODE {
public:
	INVOCATION inv;
	int seq;
	UNODE* next;
	CON_NEXT decide_next;
	UNODE(INVOCATION i, int s) : inv(i), seq(s), next(nullptr) {}

	UNODE() : seq(0), next(nullptr) {}
	void clear() {
		decide_next.clear();
		next = nullptr;
	}
};


class LFU_STD_SET {
	UNODE tail; 
	UNODE* head[MAX_THREADS];

	UNODE* max_head() {
		UNODE* max = head[0];
		for (int i = 1; i < MAX_THREADS; ++i) {
			if (max->seq < head[i]->seq)
				max = head[i];
		}
		return max;
	}
public:
	LFU_STD_SET() {
		for (int i = 0; i < MAX_THREADS; ++i)
			head[i] = &tail;
	}
	RESPONSE apply(INVOCATION inv){
		int th_id = thread_id;
		UNODE* prefer = new UNODE(inv, 0);
		while (prefer->seq == 0) {
			UNODE* before = max_head();
			UNODE* after = before->decide_next.decide(prefer);
			before->next = after;
			after->seq = before->seq + 1;
			head[th_id] = after;
		}

		SEQ_STD_SET seq_set;
		UNODE* node = tail.next;

		while (node != nullptr) {
			seq_set.apply(node->inv);
			node = node->next;
		}

		return seq_set.apply(inv);

	}

	void clear() {
		tail.clear();
		for (int i = 0; i < MAX_THREADS; ++i)
			head[i] = &tail;
	}

	void print20()
	{
		SEQ_STD_SET seq_set;
		UNODE* node = tail.next;
		while (node != nullptr) {
			seq_set.apply(node->inv);
			node = node->next;
		}
		seq_set.print20();
	}
};

class LFU_SET {
	LFU_STD_SET lfuset;
public:
	LFU_SET() {}
	~LFU_SET() {}

	void clear() {
		lfuset.clear();
	}

	bool add(int x)
	{
		RESPONSE res = lfuset.apply(INVOCATION(ADD, x));
		return res.result;
	}

	bool remove(int x)
	{
		RESPONSE res = lfuset.apply(INVOCATION(REMOVE, x));
		return res.result;
	}

	bool contains(int x)
	{
		RESPONSE res = lfuset.apply(INVOCATION(CONTAINS, x));
		return res.result;
	}

	void print20()
	{
		lfuset.print20();
	}
};