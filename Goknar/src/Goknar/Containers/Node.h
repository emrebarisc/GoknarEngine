#ifndef __NODE_H__
#define __NODE_H__

template <class T>
class SinglyNode
{
public:

private:
	T data;
	SinglyNode next;
};

template <class T>
class DoublyNode
{
public:

private:
	T data;
	DoublyNode next;
	DoublyNode prev;
};

#endif