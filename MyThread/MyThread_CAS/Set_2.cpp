#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

const int MAX_THREADS = 16;

class Node
{
public:
	int value;
	Node* next;
	Node(int val) : value(val), next(nullptr) {}

	void lock() { nodeLock.lock(); }
	void unlock() { nodeLock.unlock(); }

private:
	std::mutex nodeLock;
};


class C_SET
{
private:
	Node* head, * tail;
	std::mutex set_lock;
public:
	C_SET()
	{
		// Set의 범위는 [0, 1000]으로 제한하겠음
		head = new Node(INT_MIN);
		tail = new Node(INT_MAX);
		head->next = tail;
	}

	~C_SET()
	{
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		Node* curr = head->next;
		while (curr != tail)
		{
			Node* temp = curr;
			curr = curr->next;
			delete temp;
		}
		head->next = tail;
	}

	bool add(int x) {
		Node* pred = head;
		pred->lock();
		Node* curr = pred->next;
		//curr->lock();

		// pred->value < x <= curr->value
		while (curr->value < x) {
			//Node* next = curr->next;
			curr->lock();         // 1) 다음을 먼저 잠그고
			pred->unlock();       // 2) 이전을 푼다
			pred = curr;          // 3) 전진
			curr = curr->next;
		}

		if (curr->value == x) {
			curr->unlock();
			pred->unlock();
			return false;
		}

		Node* n = new Node(x);
		n->next = curr;
		pred->next = n;

		curr->unlock();
		pred->unlock();
		return true;
	}

	bool remove(int x) {
		Node* pred = head;
		pred->lock();
		Node* curr = pred->next;
		//curr->lock();

		while (curr->value < x) {
			curr->lock();
			pred->unlock();
			pred = curr;
			curr = curr->next;
		}

		if (curr->value != x) {
			curr->unlock();
			pred->unlock();
			return false;
		}

		// pred, curr 모두 잠긴 상태에서만 링크 수정
		pred->next = curr->next;
		curr->unlock();
		pred->unlock();
		delete curr;
		return true;
	}

	bool contains(int x) {
		Node* pred = head;
		pred->lock();
		Node* curr = pred->next;
		//curr->lock();

		while (curr->value < x) {
			pred->unlock();
			curr->lock();
			pred = curr;
			curr = curr->next;
		}

		bool found = (curr->value == x);
		curr->unlock();
		pred->unlock();
		return found;
	}
	void print20()
	{
		Node* curr = head->next;
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next)
			std::cout << curr->value << " ";
		std::cout << std::endl;
	}
};



C_SET clist;

const auto NUM_TEST = 4000000;
const auto KEY_RANGE = 1000;



void Benchmark(const int num_thread)
{
	const int loop_count = NUM_TEST / num_thread;

	int key{ 0 };

	for (int i = 0; i < loop_count; i++) {
		switch (rand() % 3) {
		case 0:
			key = rand() % KEY_RANGE;
			clist.add(key);
			break;
		case 1:
			key = rand() % KEY_RANGE;
			clist.remove(key);
			break;
		case 2:
			key = rand() % KEY_RANGE;
			clist.contains(key);
			break;
		default:
			std::cout << "Error\n";
			exit(-1);
		}
	}
}

int main()
{
	std::cout << "<C_SET 테스트>\n\n";

	using namespace std::chrono;
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2)
	{
		clist.clear();
		auto start = high_resolution_clock::now();
		std::vector<std::thread> threads;
		for (int i = 0; i < num_threads; i++)
			threads.emplace_back(Benchmark, num_threads);
		for (auto& th : threads)
			th.join();
		auto end = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end - start).count();
		std::cout << num_threads << "개의 쓰레드 실행 시간 : " << duration << "ms" << std::endl;
		std::cout << "20개 출력 결과: "; clist.print20();
	}

}