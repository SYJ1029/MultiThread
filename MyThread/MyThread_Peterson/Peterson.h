#pragma once

template <typename FLAG, typename VIC>
class Peterson
{
public:
	Peterson()
	{
		flag[0] = false;
		flag[1] = false;
		victim = 0;
	}
	~Peterson() {}
	void lock(int id)
	{
		int other = 1 - id;
		flag[id] = true;
		victim = id;
		while (flag[other] && victim == id)
		{
			// busy wait
		}
	}
	void unlock(int id)
	{
		flag[id] = false;
	}
private:
	FLAG flag[2];
	VIC victim;

};

