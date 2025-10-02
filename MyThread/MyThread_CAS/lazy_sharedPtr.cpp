#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <memory>

const int MAX_THREADS = 16;

// �̵� �ÿ� lock�� �ɷ����� �ʵ��� ����

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


class L_SET
{
private:
	std::shared_ptr<Node> head, tail;
public:
	L_SET()
	{
		// Set�� ������ [0, 1000]���� �����ϰ���
		head = std::make_shared<Node>(INT_MIN);
		tail = std::make_shared<Node>(INT_MAX);
		head->next = tail;
	}

	~L_SET()
	{
		clear();
	}

	void clear()
	{
		std::shared_ptr<Node> curr = head->next;
		while (curr != tail)
		{
			std::shared_ptr<Node> temp = curr;
			curr = temp->next;
			temp.reset();
		}
		head->next = tail;
	}

	bool add(int x) {


		while (true) {
			std::shared_ptr<Node> pred(head);
			auto curr = pred->next;

			while (curr->value < x) {
				pred = curr;
				curr = curr->next;
			}

			// ���⼭ pred�� curr�� lock�� �Ǵ�
			pred->lock();
			curr->lock();
			if (validate(pred, curr)) {
				std::shared_ptr<Node> n = std::make_shared<Node>(x);

				if (curr->value == x) {
					curr->unlock();
					pred->unlock();
					return false;
				}

				n->next = curr;
				pred->next = n;

				curr->unlock();
				pred->unlock();
				return true;
			}


			// validate ���� ��, ��� ���� �� �ٽ� �õ�
			pred->unlock();
			curr->unlock();

		}

		// while���� �������� ���⿡ �����ϴ� break���� �����Ƿ� �����ؼ��� �ȵǴ� �ڵ�
		// �̰��� �����ߴٸ� �����ΰ� �߸��� ��
		return false;
	}

	bool remove(int x) {

		while (true) {
			std::shared_ptr<Node> pred(head);
			std::shared_ptr<Node> curr = pred->next;

			while (curr->value < x) {
				pred = curr;
				curr = curr->next;
			}
			pred->lock();
			curr->lock();

			if (validate(pred, curr)) {
				if (curr->value != x) {
					curr->unlock();
					pred->unlock();
					return false;
				}

				curr->marked = true; // ���� ����

				pred->next = curr->next;
				curr->unlock();
				pred->unlock();
				return true;
			}

			pred->unlock();
			curr->unlock();
		}
		return false;
	}

	bool contains(int x) {
		while (true) {
			std::shared_ptr<Node> pred(head);
			std::shared_ptr<Node> curr = pred->next;

			while (curr->value < x) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock();
			curr->lock();
			if (validate(pred, curr)) {

				bool found = (curr->value == x);
				curr->unlock();
				pred->unlock();
				return found;
			}

			pred->unlock();
			curr->unlock();
		}

		return false;
	}

	bool validate(std::shared_ptr<Node> pred, std::shared_ptr<Node> curr) {
		return !pred->marked && !curr->marked && pred->next == curr;
	}

	void print20()
	{
		std::shared_ptr<Node> curr(head->next);
		for (int i = 0; i < 20 && curr != tail; i++, curr = curr->next)
			std::cout << curr->value << " ";
		std::cout << std::endl;
	}
};



L_SET clist;

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
	std::cout << "<C_SET �׽�Ʈ>\n\n";

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
		std::cout << num_threads << "���� ������ ���� �ð� : " << duration << "ms" << std::endl;
		std::cout << "20�� ��� ���: "; clist.print20();
	}

}