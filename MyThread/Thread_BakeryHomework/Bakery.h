#pragma once

const int MAX_THREADS = 8;
int num_threads = 1;

template <typename T, typename Y>
class Bakery {
	T* flag;
	Y* label;
public:
	Bakery() {
		label = nullptr;
		flag = nullptr;
	}
	void make(int n) {
		flag = new T[n];
		label = new Y[n];
		for (int i = 0; i < n; i++) {
			flag[i] = false;
			label[i] = 0;
		}
	}
	void destroy() {
		delete[] flag;
		delete[] label;
		flag = nullptr;
		label = nullptr;
	}
	int MaxLabel() {
		int max = label[0];
		for (int i = 1; i < num_threads; i++) {
			if (label[i] > max) {
				max = label[i];
			}
		}
		return max;
	}
	void lock(int id) {
		flag[id] = true;
		label[id] = MaxLabel() + 1;
		for (int i = 0; i < num_threads; ++i)
		{
			if (i == id) continue;
			while (flag[i] && (label[i] < label[id] || (label[i] == label[id] && i < id)))
			{
			}
		}
	}
	void unlock(int id) {
		flag[id] = false;
		label[id] = 0;
	}
};


