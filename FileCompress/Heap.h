#pragma once 
#include<iostream>

#include<vector>
using namespace std;
#include<assert.h>


template<class T>
struct Small
{
public:
	bool operator()(const T& l, const T& r)
	{
		return l < r;
	}
};
//
//template<class T>//可用来建大堆
//struct Big
//{
//	bool operator()(const T& l, const T& r)
//	{
//		return l > r;
//	}
//};

template<class T,class CompareNode=Small<T>>//建立小堆
class Heap
{
public:
	
	Heap()
	{
	}

	Heap(const T* arr,int size)
	{
		for (int i = 0; i < size; i++)
		{
			_v.push_back(arr[i]);
		}

		for (int i = _v.size() / 2-1; i>=0; i--)
		{
			_AdjustDown(i);
		}
	}

	~Heap()
	{}

	void Push(const T& d)
	{
		_v.push_back(d);
		_AdjustUp(_v.size()-1);
	}

	int Size()
	{
		return _v.size();
	}

	T& Top()
	{
		return *(_v.begin());
	}

	void Pop()//用交换法
	{
		swap(_v[0], _v[_v.size()-1]);
		_v.pop_back();
		_AdjustDown(0);
	}
protected:
	void _AdjustDown(int parent)//向下调整
	{
		CompareNode compareNode;
		int child = 2 * parent + 1;

		while (child < _v.size())
		{
			if (child + 1 < _v.size() && compareNode(_v[child + 1], _v[child]))//找较小的child
			{
				child++;
			}
			if (compareNode(_v[child], _v[parent]))
			{
				swap(_v[parent], _v[child]);
				parent = child;
				child = 2 * parent + 1;
			}
			else
				break;
		}
	}

	void _AdjustUp(int child)//向上调整
	{
		CompareNode compareNode;
		int parent = (child-1)/2;
		while (child>0)
		{
			/*if (child + 1 < _v.size() && compareNode(_v[child + 1], _v[child]))
			{
				child++;
			}
*/
			if (compareNode(_v[child], _v[parent]))
			{
				swap(_v[parent], _v[child]);
				child = parent;
				parent = (child - 1) / 2;
			}
			else
				break;
		}
	}
private:
	vector<T> _v;
};
