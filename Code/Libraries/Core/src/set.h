#ifndef SET_H
#define SET_H

// Set implemented with a red-black tree
// The Key class must implement < and == to compile this.

// This is basically just my Map class with only Keys. 
// [] operator is removed because it doesn't make sense for this.

// Red-black fixup functions are based largely on the code
// provided at http://en.wikipedia.org/wiki/Red-black_tree

#define FOR_EACH_SET( iter, set, type ) for( Set<type>::Iterator iter = (set).Begin(); iter != (set).End(); ++iter )
#define FOR_EACH_SET_NOINCR( iter, set, type ) for( Set<type>::Iterator iter = (set).Begin(); iter != (set).End(); )
#define FOR_EACH_SET_REVERSE( iter, set, type ) for( Set<type>::Iterator iter = (set).Last(); iter != (set).End(); --iter )
#define FOR_EACH_SET_REVERSE_NOINCR( iter, set, type ) for( Set<type>::Iterator iter = (set).Last(); iter != (set).End(); )

template<class C> class Set
{
private:
	template<class NC> class _Node
	{
		friend class Set;

		enum EColor
		{
			Red,
			Black,
		};

		_Node<NC>() : m_Parent( NULL ), m_Left( NULL ), m_Right( NULL ) {}

		EColor		m_Color;
		NC			m_Item;
		_Node<NC>*	m_Parent;
		_Node<NC>*	m_Left;
		_Node<NC>*	m_Right;
	};

	typedef _Node<C>	Node;

public:
	template<class IC> class _Iterator
	{
		friend class Set;

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
			DEVASSERT( m_Node );
			if( m_Node )
			{
				if( m_Node->m_Right )
				{
					Node* NewNode = m_Node->m_Right;
					while( NewNode->m_Left )
					{
						NewNode = NewNode->m_Left;
					}
					m_Node = NewNode;
				}
				else
				{
					// Find the lowest left edge
					Node* Child		= m_Node;
					Node* Parent	= m_Node->m_Parent;
					while( Parent )
					{
						if( Child == Parent->m_Left )
						{
							m_Node = Parent;
							return *this;
						}
						Child	= Parent;
						Parent	= Child->m_Parent;
					}
					m_Node = NULL;
				}
			}
			return *this;
		}

		_Iterator<IC>& operator--()
		{
			DEVASSERT( m_Node );
			if( m_Node )
			{
				if( m_Node->m_Left )
				{
					Node* NewNode = m_Node->m_Left;
					while( NewNode->m_Right )
					{
						NewNode = NewNode->m_Right;
					}
					m_Node = NewNode;
				}
				else
				{
					// Find the lowest right edge
					Node* Child		= m_Node;
					Node* Parent	= m_Node->m_Parent;
					while( Parent )
					{
						if( Child == Parent->m_Right )
						{
							m_Node = Parent;
							return *this;
						}
						Child	= Parent;
						Parent	= Child->m_Parent;
					}
					m_Node = NULL;
				}
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
	Node*			m_Root;
	uint			m_Size;
	Allocator*	m_Allocator;

	void RotateRight( Node* Pivot )
	{
		Node* LeftChild = Pivot->m_Left;
		if( LeftChild )
		{
			Node* TempParent			= Pivot->m_Parent;
			Pivot->m_Left				= LeftChild->m_Right;
			if( Pivot->m_Left )
			{
				Pivot->m_Left->m_Parent	= Pivot;
			}
			Pivot->m_Parent				= LeftChild;
			LeftChild->m_Right			= Pivot;
			LeftChild->m_Parent			= TempParent;
			if( TempParent )
			{
				if( TempParent->m_Left == Pivot )
				{
					TempParent->m_Left = LeftChild;
				}
				else
				{
					TempParent->m_Right = LeftChild;
				}
			}

			if( m_Root == Pivot )
			{
				m_Root = LeftChild;
			}
		}
	}

	void RotateLeft( Node* Pivot )
	{
		Node* RightChild = Pivot->m_Right;
		if( RightChild )
		{
			Node* TempParent				= Pivot->m_Parent;
			Pivot->m_Right					= RightChild->m_Left;
			if( Pivot->m_Right )
			{
				Pivot->m_Right->m_Parent	= Pivot;
			}
			Pivot->m_Parent					= RightChild;
			RightChild->m_Left				= Pivot;
			RightChild->m_Parent			= TempParent;
			if( TempParent )
			{
				if( TempParent->m_Left == Pivot )
				{
					TempParent->m_Left = RightChild;
				}
				else
				{
					TempParent->m_Right = RightChild;
				}
			}

			if( m_Root == Pivot )
			{
				m_Root = RightChild;
			}
		}
	}

	Node* GetGrandparent( Node* N ) const
	{
		if( N && N->m_Parent )
		{
			return N->m_Parent->m_Parent;
		}
		return NULL;
	}

	Node* GetUncle( Node* N ) const
	{
		Node* Grandparent = GetGrandparent( N );
		if( Grandparent )
		{
			if( N->m_Parent == Grandparent->m_Left )
			{
				return Grandparent->m_Right;
			}
			return Grandparent->m_Left;
		}
		return NULL;
	}

	Node* GetSibling( Node* N ) const
	{
		if( N->m_Parent )
		{
			if( N == N->m_Parent->m_Left )
			{
				return N->m_Parent->m_Right;
			}
			else
			{
				return N->m_Parent->m_Left;
			}
		}
		return NULL;
	}

	// Recursive function to maintain red-black rules
	void RBInsertFixup( Node* N )
	{
		if( N->m_Parent )
		{
			if( N->m_Parent->m_Color == Node::Red )
			{
				Node* Uncle = GetUncle( N );
				if( Uncle && Uncle->m_Color == Node::Red )
				{
					N->m_Parent->m_Color = Node::Black;
					Uncle->m_Color = Node::Black;
					Node* Grandparent = GetGrandparent( N );
					Grandparent->m_Color = Node::Red;
					RBInsertFixup( Grandparent );
				}
				else
				{
					Node* Grandparent = GetGrandparent( N );
					if( N == N->m_Parent->m_Right && N->m_Parent == Grandparent->m_Left )
					{
						RotateLeft( N->m_Parent );
						N = N->m_Left;
					}
					else if( N == N->m_Parent->m_Left && N->m_Parent == Grandparent->m_Right )
					{
						RotateRight( N->m_Parent );
						N = N->m_Right;
					}

					Grandparent = GetGrandparent( N );
					N->m_Parent->m_Color = Node::Black;
					Grandparent->m_Color = Node::Red;

					if( N == N->m_Parent->m_Left && N->m_Parent == Grandparent->m_Left )
					{
						RotateRight( Grandparent );
					}
					else
					{
						// Here, N == N->m_Parent->m_Right && N->m_Parent == Grandparent->m_Right
						RotateLeft( Grandparent );
					}
				}
			}
		}
		else
		{
			N->m_Color = Node::Black;
		}
	}

	void RBRemoveReplace( Node* N, Node* Child )
	{
		if( N->m_Parent )
		{
			if( N == N->m_Parent->m_Left )
			{
				N->m_Parent->m_Left = Child;
			}
			else
			{
				N->m_Parent->m_Right = Child;
			}

			if( Child )
			{
				Child->m_Parent = N->m_Parent;
			}
		}
		else
		{
			m_Root = Child;

			if( Child )
			{
				Child->m_Parent = NULL;
			}
		}
	}

	void RBRemove( Node* N )
	{
		// Precondition: At most one of N's children is non-null
		Node* Child = N->m_Left ? N->m_Left : N->m_Right;

		RBRemoveReplace( N, Child );

		if( Child && N->m_Color == Node::Black )
		{
			if( Child->m_Color == Node::Red )
			{
				Child->m_Color = Node::Black;
			}
			else
			{
				RBRemoveFixup( Child );
			}
		}

		delete N;
		--m_Size;
	}

	bool IsBlack( Node* N )
	{
		return ( !N || N->m_Color == Node::Black );
	}

	bool IsRed( Node* N )
	{
		return ( N && N->m_Color == Node::Red );
	}

	void RBRemoveFixup( Node* N )
	{
		if( N->m_Parent )
		{
			Node* Sibling = GetSibling( N );
			if( IsRed( Sibling ) )
			{
				N->m_Parent->m_Color	= Node::Red;
				Sibling->m_Color		= Node::Black;
				if( N == N->m_Parent->m_Left )
				{
					RotateLeft( N->m_Parent );
				}
				else
				{
					RotateRight( N->m_Parent );
				}
			}

			Sibling = GetSibling( N );
			if( N->m_Parent->m_Color == Node::Black &&
				IsBlack( Sibling ) &&
				( Sibling && IsBlack( Sibling->m_Left ) ) &&
				( Sibling && IsBlack( Sibling->m_Right ) ) )
			{
				Sibling->m_Color		= Node::Red;
				N->m_Parent->m_Color	= Node::Black;
			}
			else
			{
				if( N == N->m_Parent->m_Left &&
					IsBlack( Sibling ) &&
					( Sibling && IsRed( Sibling->m_Left ) ) &&
					( Sibling && IsBlack( Sibling->m_Right ) ) )
				{
					Sibling->m_Color			= Node::Red;
					Sibling->m_Left->m_Color	= Node::Black;
					RotateRight( Sibling );
				}
				else if( N == N->m_Parent->m_Right &&
					IsBlack( Sibling ) &&
					( Sibling && IsBlack( Sibling->m_Left ) ) &&
					( Sibling && IsRed( Sibling->m_Right ) ) )
				{
					Sibling->m_Color			= Node::Red;
					Sibling->m_Right->m_Color	= Node::Black;
					RotateLeft( Sibling );
				}

				Sibling = GetSibling( N );
				if( Sibling )
				{
					Sibling->m_Color = N->m_Parent->m_Color;
				}
				N->m_Parent->m_Color = Node::Black;
				if( N == N->m_Parent->m_Left )
				{
					if( Sibling && Sibling->m_Right )
					{
						Sibling->m_Right->m_Color = Node::Black;
					}
					RotateLeft( N->m_Parent );
				}
				else
				{
					if( Sibling && Sibling->m_Left )
					{
						Sibling->m_Left->m_Color = Node::Black;
					}
					RotateRight( N->m_Parent );
				}
			}
		}
	}

	void RecursiveClear( Node* N )
	{
		if( N )
		{
			RecursiveClear( N->m_Left );
			RecursiveClear( N->m_Right );
			delete N;
		}
	}

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

public:
	Set( Allocator* pAllocator = NULL )
		:	m_Root( NULL )
		,	m_Size( 0 )
		,	m_Allocator( pAllocator )
	{
	}

	Set( const Set& Other, Allocator* pAllocator = NULL )
		:	m_Root( NULL )
		,	m_Size( 0 )
		,	m_Allocator( pAllocator )
	{
		for( Set<C>::Iterator OtherIterator = Other.Begin(); OtherIterator != Other.End(); ++OtherIterator )
		{
			Insert( *OtherIterator );
		}
	}

	~Set()
	{
		Clear();
	};

	Set& operator=( const Set& Other )
	{
		Clear();
		for( Set<C>::Iterator OtherIterator = Other.Begin(); OtherIterator != Other.End(); ++OtherIterator )
		{
			Insert( *OtherIterator );
		}
		return *this;
	}

	Iterator Insert( const C& Item )
	{
		if( m_Root )
		{
			Node*	Parent		= m_Root;
			Node**	ChildPtr	= NULL;
			Node*	NewNode		= NULL;
			while( !NewNode )
			{
				if( Item == Parent->m_Item )
				{
					// Key already exists--just return the iterator
					return Iterator( Parent );
				}

				if( Item < Parent->m_Item )
				{
					ChildPtr = &Parent->m_Left;
				}
				else
				{
					ChildPtr = &Parent->m_Right;
				}

				if( *ChildPtr )
				{
					Parent = *ChildPtr;
				}
				else
				{
					NewNode				= *ChildPtr = AllocateNode();
					NewNode->m_Parent	= Parent;
					NewNode->m_Color	= Node::Red;
					NewNode->m_Item		= Item;
					++m_Size;
				}
			}

			RBInsertFixup( NewNode );
			return Iterator( NewNode );
		}
		else
		{
			// This is the first node in the tree
			m_Root			= AllocateNode();
			m_Root->m_Color	= Node::Black;
			m_Root->m_Item	= Item;
			++m_Size;
			return Iterator( m_Root );
		}
	}

	Iterator Search( const C& Item ) const
	{
		Node* SearchNode = m_Root;
		while( SearchNode )
		{
			if( Item == SearchNode->m_Item )
			{
				return Iterator( SearchNode );
			}
			
			if( Item < SearchNode->m_Item )
			{
				SearchNode = SearchNode->m_Left;
			}
			else
			{
				SearchNode = SearchNode->m_Right;
			}
		}
		return Iterator( NULL );
	}

	void Remove( const C& Item )
	{
		Iterator Iter( Search( Item ) );
		if( !Iter.IsNull() )
		{
			Remove( Iter );
		}
	}

	void Remove( Iterator& Iter )
	{
		Node* ToRemove = Iter.m_Node;
		++Iter;
		DEVASSERT( ToRemove );

		if( !ToRemove->m_Left || !ToRemove->m_Right )
		{
			RBRemove( ToRemove );
		}
		else
		{
			// Find in-order predecessor
			Node* Predecessor = ToRemove->m_Left;
			while( Predecessor->m_Right )
			{
				Predecessor = Predecessor->m_Right;
			}

			// Copy to the original node
			ToRemove->m_Item	= Predecessor->m_Item;

			RBRemove( Predecessor );
		}
	}

	void Clear()
	{
		RecursiveClear( m_Root );
		m_Root = NULL;
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

	Iterator Root() const
	{
		return Iterator( m_Root );
	}

	Iterator Begin() const
	{
		return First();
	}

	// Return first item for inorder traversal
	Iterator First() const
	{
		if( !m_Root )
		{
			return Iterator( NULL );
		}

		Node* Iter = m_Root;
		while( Iter->m_Left )
		{
			Iter = Iter->m_Left;
		}
		return Iterator( Iter );
	}

	// Return last item for inorder traversal
	Iterator Last() const
	{
		if( !m_Root )
		{
			return Iterator( NULL );
		}

		Node* Iter = m_Root;
		while( Iter->m_Right )
		{
			Iter = Iter->m_Right;
		}
		return Iterator( Iter );
	}

	Iterator End() const
	{
		return Iterator( NULL );
	}
};

#endif // SET_H