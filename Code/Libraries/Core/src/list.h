#ifndef LIST_H
#define LIST_H

// Doubly-linked list

#define FOR_EACH_LIST( iter, list, type ) for( List<type>::Iterator iter = (list).Begin(); iter != (list).End(); ++iter )
#define FOR_EACH_LIST_NOINCR( iter, list, type ) for( List<type>::Iterator iter = (list).Begin(); iter != (list).End(); )
#define FOR_EACH_LIST_REVERSE( iter, list, type ) for( List<type>::Iterator iter = (list).Back(); iter != (list).End(); --iter )
#define FOR_EACH_LIST_REVERSE_NOINCR( iter, list, type ) for( List<type>::Iterator iter = (list).Back(); iter != (list).End(); )

template<class C> class List
{
private:
	template<class NC> class _Node
	{
		friend class List;

		_Node<NC>() : m_Next( NULL ), m_Prev( NULL ) {}

		NC			m_Item;
		_Node<NC>*	m_Next;
		_Node<NC>*	m_Prev;
	};

	typedef _Node<C>	Node;

public:
	template<class IC> class _Iterator
	{
		friend class List;

	public:
		_Iterator<IC>() : m_Node( NULL ) {}
		_Iterator<IC>( Node* Node ) : m_Node( Node ) {}
		_Iterator<IC>( const _Iterator<IC>& Other ) : m_Node( Other.m_Node ) {}

		bool operator==( const _Iterator<IC>& Other ) const
		{
			return m_Node == Other.m_Node;
		}

		bool operator!=( const _Iterator<IC>& Other ) const
		{
			return m_Node != Other.m_Node;
		}

		_Iterator<IC>& operator=( const _Iterator<IC>& Other )
		{
			m_Node = Other.m_Node;
			return *this;
		}

		IC& operator*() const
		{
			DEVASSERT( m_Node );
			return m_Node->m_Item;
		}

		// Same as operator*
		IC& GetValue() const
		{
			DEVASSERT( m_Node );
			return m_Node->m_Item;
		}

		// Prefix
		_Iterator<IC>& operator++()
		{
			if( m_Node )
			{
				m_Node = m_Node->m_Next;
			}
			return *this;
		}

		_Iterator<IC>& operator--()
		{
			if( m_Node )
			{
				m_Node = m_Node->m_Prev;
			}
			return *this;
		}

		// Postfix
		const _Iterator<IC> operator++( int )
		{
			const _Iterator<IC> OldValue = *this;
			++( *this );
			return OldValue;
		}

		const _Iterator<IC> operator--( int )
		{
			const _Iterator<IC> OldValue = *this;
			--( *this );
			return OldValue;
		}

		bool IsNull() const
		{
			return m_Node == NULL;
		}

		bool IsValid() const
		{
			return m_Node != NULL;
		}

	private:
		Node*	m_Node;
	};

	typedef _Iterator<C>	Iterator;

private:
	Node*			m_Head;
	Node*			m_Tail;
	uint			m_Size;
	Allocator*	m_Allocator;

public:
	List( Allocator* pAllocator = NULL )
		:	m_Head( NULL )
		,	m_Tail( NULL )
		,	m_Size( 0 )
		,	m_Allocator( pAllocator )
	{
	}

	List( const List& Other, Allocator* pAllocator = NULL )
		:	m_Head( NULL )
		,	m_Tail( NULL )
		,	m_Size( 0 )
		,	m_Allocator( pAllocator )
	{
		for( List<C>::Iterator OtherIterator = Other.Begin(); OtherIterator != Other.End(); ++OtherIterator )
		{
			PushBack( *OtherIterator );
		}
	}

	~List()
	{
		Clear();
	};

	List& operator=( const List& Other )
	{
		Clear();
		for( List<C>::Iterator OtherIterator = Other.Begin(); OtherIterator != Other.End(); ++OtherIterator )
		{
			PushBack( *OtherIterator );
		}
		return *this;
	}

	void PushBack( const C& Item )
	{
		Node* const NewNode = AllocateNode();
		DEVASSERT( NewNode );

		NewNode->m_Item = Item;
		NewNode->m_Next = NULL;
		NewNode->m_Prev = m_Tail;
		if( m_Tail )
		{
			m_Tail->m_Next = NewNode;
		}
		m_Tail = NewNode;
		if( !m_Head )
		{
			m_Head = NewNode;
		}
		++m_Size;
	}

	void PushFront( const C& Item )
	{
		Node* const NewNode = AllocateNode();
		DEVASSERT( NewNode );

		NewNode->m_Item = Item;
		NewNode->m_Next = m_Head;
		NewNode->m_Prev = NULL;
		if( m_Head )
		{
			m_Head->m_Prev = NewNode;
		}
		m_Head = NewNode;
		if( !m_Tail )
		{
			m_Tail = NewNode;
		}
		++m_Size;
	}

	void PushAfter( const C& Item, Node* const pNode )
	{
		DEVASSERT( pNode );
		Node* const NewNode = AllocateNode();
		DEVASSERT( NewNode );

		NewNode->m_Item = Item;
		NewNode->m_Next = pNode->m_Next;
		NewNode->m_Prev = pNode;
		if( pNode->m_Next )
		{
			pNode->m_Next->m_Prev = NewNode;
		}
		pNode->m_Next = NewNode;
		++m_Size;
	}

	void PushAfter( const C& Item, const Iterator& Iter )
	{
		PushAfter( Item, Iter.m_Node );
	}

	void PushBefore( const C& Item, Node* const pNode )
	{
		DEVASSERT( pNode );
		Node* const NewNode = AllocateNode();
		DEVASSERT( NewNode );

		NewNode->m_Item = Item;
		NewNode->m_Next = pNode;
		NewNode->m_Prev = pNode->m_Prev;
		if( pNode->m_Prev )
		{
			pNode->m_Prev->m_Next = NewNode;
		}
		pNode->m_Prev = NewNode;
		++m_Size;
	}

	void PushBefore( const C& Item, const Iterator& Iter )
	{
		PushBefore( Item, Iter.m_Node );
	}

	void PopBack()
	{
		if( m_Tail )
		{
			Node* NewTail = m_Tail->m_Prev;
			delete m_Tail;
			m_Tail = NewTail;
			if( !m_Tail )
			{
				m_Head = NULL;
			}
			else
			{
				m_Tail->m_Next = NULL;
			}
			--m_Size;
		}
	}

	void PopFront()
	{
		if( m_Head )
		{
			Node* NewHead = m_Head->m_Next;
			delete m_Head;
			m_Head = NewHead;
			if( !m_Head )
			{
				m_Tail = NULL;
			}
			else
			{
				m_Head->m_Prev = NULL;
			}
			--m_Size;
		}
	}

private:
	// Can be the head, tail, or any item in the middle, so has to handle many cases
	void Pop( Node* PopNode )
	{
		DEVASSERT( PopNode );
		if( PopNode == m_Head )
		{
			PopFront();
		}
		else if( PopNode == m_Tail )
		{
			PopBack();
		}
		else
		{
			if( PopNode->m_Next )
			{
				PopNode->m_Next->m_Prev = PopNode->m_Prev;
			}
			if( PopNode->m_Prev )
			{
				PopNode->m_Prev->m_Next = PopNode->m_Next;
			}
			delete PopNode;
			--m_Size;
		}
	}

public:
	// NOTE: Advances the iterator to the next node
	void Pop( Iterator& Iter )
	{
		Node* ToPop = Iter.m_Node;
		++Iter;
		DEVASSERT( ToPop );
		Pop( ToPop );
	}

	// NOTE: Requires C::operator== to be implemented
	// Linear time to find the item(s)
	void Remove( const C& Item )
	{
		Node* IterNode = m_Head;
		while( IterNode )
		{
			Node* Next = IterNode->m_Next;
			if( IterNode->m_Item == Item )
			{
				Pop( IterNode );
			}
			IterNode = Next;
		}
	}

	void Clear()
	{
		Node* ToDelete;
		Node* Next;
		for( ToDelete = m_Head; ToDelete; ToDelete = Next )
		{
			Next = ToDelete->m_Next;
			delete ToDelete;
		}
		m_Head = NULL;
		m_Tail = NULL;
		m_Size = 0;
	}

	uint Size() const
	{
		return m_Size;
	}

	bool Empty() const
	{
		return m_Size == 0;
	}

	// NOTE: Requires C::operator< to be implemented
	// Uses merge sort (O(n log n)) (my usual choice of
	// quick sort is better for arrays than lists, it seems)
	void Sort()
	{
		MergeSort();
	}

private:
	// In-place merge sort
	void MergeSort()
	{
		if( m_Size <= 1 )
		{
			return;
		}

		List Left, Right;
		uint Middle = m_Size >> 1;
		Node* IterNode = m_Head;
		uint i = 0;
		for( ; i < Middle; ++i, IterNode = IterNode->m_Next )
		{
			Left.PushFront( IterNode->m_Item );
		}
		for( ; i < m_Size; ++i, IterNode = IterNode->m_Next )
		{
			Right.PushFront( IterNode->m_Item );
		}

		Left.MergeSort();
		Right.MergeSort();
		
		Node* LeftNode = Left.m_Head;
		Node* RightNode = Right.m_Head;
		IterNode = m_Head;
		while( LeftNode && RightNode )
		{
			if( LeftNode->m_Item < RightNode->m_Item )
			{
				IterNode->m_Item = LeftNode->m_Item;
				LeftNode = LeftNode->m_Next;
			}
			else
			{
				IterNode->m_Item = RightNode->m_Item;
				RightNode = RightNode->m_Next;
			}
			IterNode = IterNode->m_Next;
		}
		while( LeftNode )
		{
			IterNode->m_Item = LeftNode->m_Item;
			LeftNode = LeftNode->m_Next;
			IterNode = IterNode->m_Next;
		}
		while( RightNode )
		{
			IterNode->m_Item = RightNode->m_Item;
			RightNode = RightNode->m_Next;
			IterNode = IterNode->m_Next;
		}
	}

public:
	Iterator Front() const
	{
		return Iterator( m_Head );
	}

	Iterator Back() const
	{
		return Iterator( m_Tail );
	}

	Iterator Begin() const
	{
		return Iterator( m_Head );
	}

	Iterator End() const
	{
		return Iterator( NULL );
	}

private:
	Node*	AllocateNode()
	{
		if( m_Allocator )
		{
			return new( *m_Allocator ) Node;
		}
		else
		{
			return new Node;
		}
	}
};

#endif // LIST_H