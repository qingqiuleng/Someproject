#pragma once 
#include<iostream>
#include"Heap.h"
using namespace std;


template<class T>
struct HuffManTreeNode
{
	HuffManTreeNode(const T& weight)
	:_left(NULL)
	,_right(NULL)
	,_weight(weight)
	{
	}
	HuffManTreeNode<T>* _left;
	HuffManTreeNode<T>* _right;
	T _weight;
};


template<class T>
class HuffManTree
{
public:
	typedef HuffManTreeNode<T> Node;
public:

	HuffManTree(T* arr, int size, T& invalid)//建立一个小堆
	{
		struct CompareNode
		{
			bool operator()(Node*& L,Node*& R)
			{
				return L->_weight < R->_weight;
			}
		};
		Heap<Node*, CompareNode> MinHeap;

		for (int i = 0; i < size; i++)
		{
			if (arr[i]!=invalid)
				MinHeap.Push(new Node(arr[i]));
		}
						
		while (MinHeap.Size()>1)
		{
			Node* left = MinHeap.Top();
			MinHeap.Pop();
			Node* right = MinHeap.Top();
			MinHeap.Pop();
			Node* parent = new Node(left->_weight + right->_weight);
			parent->_left = left;
			parent->_right = right;
			MinHeap.Push(parent);
		}

		_root = MinHeap.Top();
		MinHeap.Pop();
	}

	Node* _GetRoot()
	{
		return _root;
	}

private:
	Node* _root;
};			

