#ifndef MAP_H
#define MAP_H

// Map (associative array) implemented with a red-black tree
// The Key class must implement < and == to compile this.

// Red-black fixup functions are based largely on the code
// provided at http://en.wikipedia.org/wiki/Red-black_tree

#define FOR_EACH_MAP( iter, map, ktype, vtype ) for( Map<ktype, vtype>::Iterator iter = (map).Begin(); iter != (map).End(); ++iter )
#define FOR_EACH_MAP_NOINCR( iter, map, ktype, vtype ) for( Map<ktype, vtype>::Iterator iter = (map).Begin(); iter != (map).End(); )
#define FOR_EACH_MAP_REVERSE( iter, map, ktype, vtype ) for( Map<ktype, vtype>::Iterator iter = (map).Last(); iter != (map).End(); --iter )
#define FOR_EACH_MAP_REVERSE_NOINCR( iter, map, ktype, vtype ) for( Map<ktype, vtype>::Iterator iter = (map).Last(); iter != (map).End(); )

template<class K, class V> class Map
{
private:
	template<class NK, class NV> class _Node
	{
		friend class Map;

		enum EColor
		{
			Red,
			Black,
		};

		_Node<NK, NV>() : m_Parent( NULL ), m_Left( NULL ), m_Right( NULL ) {}

		EColor			m_Color;
		NK				m_Key;
		NV				m_Value;
		_Node<NK, NV>*	m_Parent;
		_Node<NK, NV>*	m_Left;
		_Node<NK, NV>*	m_Right;
	};

	typedef _Node<K, V>	Node;

public:
	template<class IK, class IV> class _Iterator
	{
		friend class Map;

	public:
		_Iterator<IK, IV>() : m_Node( NULL ) {}
		_Iterator<IK, IV>( Node* Node ) : m_Node( Node ) {}
		_Iterator<IK, IV>( const _Iterator<IK, IV>& Other ) : m_Node( Other.m_Node ) {}

		bool operator==( const _Iterator<IK, IV>& Other ) const
		{
			return m_Node == Other.m_Node;
		}

		bool operator!=( const _Iterator<IK, IV>& Other ) const
		{
			return m_Node != Other.m_Node;
		}

		_Iterator<IK, IV>& operator=( const _Iterator<IK, IV>& Other )
		{
			m_Node = Other.m_Node;
			return *this;
		}

		IV& operator*() const
		{
			DEVASSERT( m_Node );
			return m_Node->m_Value;
		}

		// Useful when iterating the map to get back to the key
		IK& GetKey() const
		{
			DEVASSERT( m_Node );
			return m_Node->m_Key;
		}

		// Same as operator*
		IV& GetValue() const
		{
			DEVASSERT( m_Node );
			return m_Node->m_Value;
		}

		// Prefix
		_Iterator<IK, IV>& operator++()
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

		_Iterator<IK, IV>& operator--()
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
		const _Iterator<IK, IV> operator++( int )
		{
			const _Iterator<IK, IV> OldValue = *this;
			++( *this );
			return OldValue;
		}

		const _Iterator<IK, IV> operator--( int )
		{
			const _Iterator<IK, IV> OldValue = *this;
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

	typedef _Iterator<K, V>	Iterator;

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

		FreeNode( N );
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
			FreeNode( N );
		}
	}

	inline Node* AllocateNode()
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

	inline void FreeNode( Node* const pNode )
	{
		delete pNode;
	}

public:
	Map( Allocator* pAllocator = NULL )
		:	m_Root( NULL )
		,	m_Size( 0 )
		,	m_Allocator( pAllocator )
	{
	}

	Map( const Map& Other, Allocator* pAllocator = NULL )
		:	m_Root( NULL )
		,	m_Size( 0 )
		,	m_Allocator( pAllocator )
	{
		for( Map<K, V>::Iterator OtherIterator = Other.Begin(); OtherIterator != Other.End(); ++OtherIterator )
		{
			Insert( OtherIterator.GetKey(), OtherIterator.GetValue() );
		}
	}

	~Map()
	{
		Clear();
	};

	Map& operator=( const Map& Other )
	{
		Clear();
		for( Map<K, V>::Iterator OtherIterator = Other.Begin(); OtherIterator != Other.End(); ++OtherIterator )
		{
			Insert( OtherIterator.GetKey(), OtherIterator.GetValue() );
		}
		return *this;
	}

	Iterator Insert( const K& Key, const V& Value )
	{
		if( m_Root )
		{
			Node*	Parent		= m_Root;
			Node**	ChildPtr	= NULL;
			Node*	NewNode		= NULL;
			while( !NewNode )
			{
				if( Key == Parent->m_Key )
				{
					// Key already exists--return the iterator and *do* replace the value
					// (Old behavior was to not replace the value, but in practice, I want that)
					Parent->m_Value = Value;
					return Iterator( Parent );
				}

				if( Key < Parent->m_Key )
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
					NewNode->m_Key		= Key;
					NewNode->m_Value	= Value;
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
			m_Root->m_Key	= Key;
			m_Root->m_Value	= Value;
			++m_Size;
			return Iterator( m_Root );
		}
	}

	V& Insert( const K& Key )
	{
		return *Insert( Key, V() );
	}

	Iterator Search( const K& Key ) const
	{
		Node* SearchNode = m_Root;
		while( SearchNode )
		{
			if( Key == SearchNode->m_Key )
			{
				return Iterator( SearchNode );
			}
			
			if( Key < SearchNode->m_Key )
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

	// Returns the first node that matches the value
	// Slow like linked list and requires operator== to be implemented for V
	Iterator SearchValue( const V& Value ) const
	{
		for( Iterator Iter = Begin(); Iter != End(); ++Iter )
		{
			if( Iter.GetValue() == Value )
			{
				return Iter;
			}
		}
		return Iterator( NULL );
	}

	void Remove( const K& Key )
	{
		Iterator Iter( Search( Key ) );
		if( !Iter.IsNull() )
		{
			Remove( Iter );
		}
	}

	// Safe to do during iteration as long as Iter is the only iterator over this Map
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
			ToRemove->m_Key		= Predecessor->m_Key;
			ToRemove->m_Value	= Predecessor->m_Value;

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

	// NOTE: Side effect: creates the entry if the key is not found
	V& operator[]( const K& Key )
	{
		Iterator Iter = Search( Key );
		if( Iter.IsNull() )
		{
			Iter = Insert( Key, V() );	// Add the default value if it's not found (so the syntax MyMap[MyKey] = MyValue will work)
		}
		return *Iter;
	}

	// NOTE: Fails if the key is not found
	V& operator[]( const K& Key ) const
	{
		Iterator Iter = Search( Key );
		DEVASSERT( Iter.IsValid() );
		return *Iter;
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

#endif // MAP_H