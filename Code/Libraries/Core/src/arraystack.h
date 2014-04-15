#ifndef ARRAYSTACK_H
#define ARRAYSTACK_H

#include "array.h"

// Should be functionally equivalent to Stack, but using a non-shrinking array instead of a list.
// Better for heap performance!

template<class C> class ArrayStack
{
private:
	Array<C>		m_Array;

public:
	ArrayStack( Allocator* pAllocator = NULL )
		:	m_Array( pAllocator )
	{
		m_Array.SetDeflate( false );
	}

	ArrayStack( const ArrayStack& Other, Allocator* pAllocator = NULL )
		:	m_Array( Other.m_Array, pAllocator )
	{
		m_Array.SetDeflate( false );
	}
	
	~ArrayStack()
	{
	}

	ArrayStack& operator=( const ArrayStack& Other )
	{
		m_Array = Other.m_Array;
		return *this;
	}

	void Push( const C& Item )
	{
		m_Array.PushBack( Item );
	}

	void Pop()
	{
		m_Array.PopBack();
	}

	C& Top()
	{
		return m_Array[ m_Array.Size() - 1 ];
	}

	void Clear()
	{
		m_Array.Clear();
	}

	uint Size()
	{
		return m_Array.Size();
	}

	bool Empty()
	{
		return m_Array.Size() == 0;
	}
};

#endif // ARRAYSTACK_H