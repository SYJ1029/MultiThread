#pragma once


#include "pch.h"

typedef class LF_NODE_EPOCH;

class MR {
	volatile long long ptr_and_mark;
public:
	MR(LF_NODE_EPOCH* p)
	{
		ptr_and_mark = reinterpret_cast<long long>(p);
	}
	MR() : ptr_and_mark(0) {}
	LF_NODE_EPOCH* get_ptr()
	{
		long long temp = ptr_and_mark & 0xFFFFFFFFFFFFFFFE;
		return reinterpret_cast<LF_NODE_EPOCH*>(temp);
	}
	bool get_mark() { return (ptr_and_mark & 1) == 1; }
	void set_ptr(LF_NODE_EPOCH* p) {
		ptr_and_mark = reinterpret_cast<long long>(p);
	}
	LF_NODE_EPOCH* get_ptr_and_mark(bool* mark) {
		long long temp = ptr_and_mark;
		*mark = ((temp & 1) == 1);
		temp = temp & 0xFFFFFFFFFFFFFFFE;
		return reinterpret_cast<LF_NODE_EPOCH*>(temp);
	}
	bool CAS(LF_NODE_EPOCH* old_ptr, LF_NODE_EPOCH* new_ptr,
		bool old_mark, bool new_mark)
	{
		long long o = reinterpret_cast<long long>(old_ptr);
		if (old_mark) o = o | 1;
		long long n = reinterpret_cast<long long>(new_ptr);
		if (new_mark) n = n | 1;
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<volatile std::atomic<long long>*>(&ptr_and_mark),
			&o, n);
	}
};

typedef class LF_NODE_EPOCH {
public:
	int value;
	MR next;
	volatile int epoch;
	LF_NODE_EPOCH(int v) : value(v) {}
}LF_NODE;

