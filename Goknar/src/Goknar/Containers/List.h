#ifndef __LIST_H__
#define __LIST_H__

#include "Node.h"

template <class T>
class List
{
public:
	List() : head_(new SinglyNode<T>)
	{}

	~List()
	{
		delete head_;
	}

	SinglyNode<T>* GetHead()
	{
		return head_;
	}

protected:

private:
	SinglyNode<T>* head_;
};


template <class T>
class CircularList
{
public:

protected:

private:

};

#endif