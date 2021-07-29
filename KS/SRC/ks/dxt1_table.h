
#ifndef INCLUDED_DXT1_TABLE_H
#define INCLUDED_DXT1_TABLE_H

#include "global.h"

template <class Type> class Table
{
private:
	enum { MAX_SIZE = 16 };

private:
	Type	data[MAX_SIZE];
	int		size;

public:
	// Creators.
	Table() { size = 0; }
	Table(const Table & t);
	virtual ~Table() { }

	// Modifiers.
	void SetCount(const int n) { assert(n <= MAX_SIZE); size = n; }
	void Append(const Type & elem);
	void Resize(const int n);

	// Accessors.
	int Count(void) const { return size; }

	// Operators.
	Table & operator=(const Table & t);
	Type & operator[](const int i);
};

//	Table()
// Copy constructor.
template <class Type>
Table<Type>::Table(const Table & t)
{
	size = t.size;
	for (int i = 0; i < size; i++)
		data[i] = t.data[i];
}

//	Append()
// Append an element position at end of array.
template <class Type>
void Table<Type>::Append(const Type & elem)
{
    assert(size < MAX_SIZE);
	data[size++] = elem;
}

//	Resize()
// Change size of array.
template <class Type>
void Table<Type>::Resize(const int n)
{
    assert(n >= 0 && n <= MAX_SIZE);
	size = n;
}

//	operator=()
// Assignment operator.
template <class Type>
Table<Type> & Table<Type>::operator=(const Table<Type> & t)
{
	if (this != &t)
	{
		size = t.size;
		for (int i = 0; i < size; i++)
			data[i] = t.data[i];
	}

	return *this;
}

//	operator[]
// Array operator.
template <class Type>
Type & Table<Type>::operator[](const int i)
{
	assert(i >= 0 && i < size);
	return data[i];
}

#endif INCLUDED_DXT1_TABLE_H