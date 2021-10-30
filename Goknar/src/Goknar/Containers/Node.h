#ifndef __NODE_H__
#define __NODE_H__

template <class T>
class SinglyNode
{
public:
	SinglyNode() :
		next(nullptr)
	{

	}

	T data;
	SinglyNode* next;
};

template <class T>
class DoublyNode
{
public:
	DoublyNode() :
		next(nullptr),
		prev(nullptr)
	{

	}

	T data;
	DoublyNode* next;
	DoublyNode* prev;
};

#endif