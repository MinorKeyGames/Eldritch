#ifndef STACK_H
#define STACK_H

// Uses a linked list implementation

template<class C> class Stack
{
private:
	template<class NC> class _StackNode
	{
		friend class Stack;

	public:
		_StackNode<NC>() : m_Next( NULL ) {}

		NC				m_Item;
		_StackNode<NC>*	m_Next;
	};

	typedef _StackNode<C>	StackNode;

	StackNode*		m_Top;
	uint			m_Size;
	Allocator*	m_Allocator;

public:
	Stack( Allocator* pAllocator = NULL )
		:	m_Top( NULL )
		,	m_Size( 0 )
		,	m_Allocator( pAllocator )
	{
	}

	Stack( const Stack& Other, Allocator* pAllocator = NULL )
		:	m_Top( NULL )
		,	m_Size( 0 )
		,	m_Allocator( pAllocator )
	{
		WARNDESC( "Stack copy constructor not yet implemented" );
	}
	
	~Stack()
	{
		Clear();
	}

	Stack& operator=( const Stack& Other )
	{
		WARNDESC( "Stack assignment not yet implemented" );
		return *this;
	}

	void Push( const C& Item )
	{
		StackNode* NewNode = AllocateNode();
		NewNode->m_Item = Item;
		NewNode->m_Next = m_Top;
		m_Top = NewNode;
		++m_Size;
	}

	void Pop()
	{
		if( m_Top )
		{
			StackNode* NewTop = m_Top->m_Next;
			delete m_Top;
			m_Top = NewTop;
			--m_Size;
		}
	}

	C& Top() const
	{
		return m_Top->m_Item;
	}

	void Clear()
	{
		StackNode* ToDelete;
		StackNode* Next;
		for( ToDelete = m_Top; ToDelete; ToDelete = Next )
		{
			Next = ToDelete->m_Next;
			delete ToDelete;
		}
		m_Top = NULL;
		m_Size = 0;
	}

	uint Size()
	{
		return m_Size;
	}

	bool Empty()
	{
		return m_Size == 0;
	}

private:
	StackNode*	AllocateNode()
	{
		if( m_Allocator )
		{
			return new( *m_Allocator ) StackNode;
		}
		else
		{
			return new StackNode;
		}
	}
};

#endif // STACK_H