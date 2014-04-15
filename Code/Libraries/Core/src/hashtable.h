#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "hashedstring.h"

// Pairs a hashed string with any class in a chained hash table
// (For arbitrarily-typed key-value pairs, write a red-black tree)

#define DEFAULT_NUM_SLOTS 16

template<class C> class HashTable
{
private:
	template<class NC> class _Node
	{
		friend class HashTable;

		_Node<NC>() : m_Next( NULL ) {}

		HashedString	m_Key;
		NC				m_Item;
		_Node<NC>*		m_Next;
	};

	typedef _Node<C>	Node;

	Node**			m_Table;
	uint			m_Size;		// Number of actual entries
	uint			m_NumSlots;	// Size of array
	Allocator*	m_Allocator;

public:
	template<class IC> class _Iterator
	{
		friend class HashTable;

	public:
		_Iterator<IC>() : m_Node( NULL ) {}
		_Iterator<IC>( Node* Node ) : m_Node( Node ) {}

		bool operator==( const _Iterator<IC>& Other ) const
		{
			return m_Node == Other.m_Node;
		}

		bool operator!=( const _Iterator<IC>& Other ) const
		{
			return m_Node != Other.m_Node;
		}

		void operator=( const _Iterator<IC>& Other )
		{
			m_Node = Other.m_Node;
		}

		IC& operator*() const
		{
			ASSERT( m_Node );
			return m_Node->m_Item;
		}

		// Prefix
		_Iterator<IC>& operator++()
		{
			ASSERT( m_Node );
			m_Node = m_Node->m_Next;
			return *this;
		}

		// Postfix
		const _Iterator<IC> operator++( int )
		{
			const _Iterator<IC> OldValue = *this;
			++( *this );
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

	HashTable( Allocator* pAllocator = NULL )
		:	m_Table( NULL )
		,	m_Size( 0 )
		,	m_NumSlots( DEFAULT_NUM_SLOTS )
		,	m_Allocator( pAllocator )
	{
		Initialize();
	}

	HashTable( uint NumSlots, Allocator* pAllocator = NULL )
		:	m_Table( NULL )
		,	m_Size( 0 )
		,	m_NumSlots( NumSlots )
		,	m_Allocator( pAllocator )
	{
		Initialize();
	}

	HashTable( const HashTable& Other, Allocator* pAllocator = NULL )
		:	m_Table( NULL )
		,	m_Size( 0 )
		,	m_NumSlots( DEFAULT_NUM_SLOTS )
		,	m_Allocator( pAllocator )
	{
		Initialize();
		WARNDESC( "Hash table copy constructor not yet implemented" );
	}

	~HashTable()
	{
		SafeDeleteArray( m_Table );
	}

	HashTable& operator=( const HashTable& Other )
	{
		WARNDESC( "Hash table assignment not yet implemented" );
		return *this;
	}

private:
	void Initialize()
	{
		ASSERT( m_Table == NULL );
		m_Table = AllocateNodeArray();
		memset( m_Table, 0, sizeof( Node* ) * m_NumSlots );
	}

public:
	void Insert( const HashedString& Key, const C& Item )
	{
#if BUILD_DEBUG
		Iterator CheckKeyExists = Search( Key );
		ASSERT( CheckKeyExists.IsNull() );
#endif

		// Iterate over the chain until the key or a null node is found
		uint Index			= Key.GetHash() % m_NumSlots;
		Node* NewNode		= AllocateNode();
		NewNode->m_Key		= Key;
		NewNode->m_Item		= Item;
		NewNode->m_Next		= m_Table[ Index ];
		m_Table[ Index ]	= NewNode;
		++m_Size;
	}

	Iterator Search( const HashedString& Key ) const
	{
		uint Index = Key.GetHash() % m_NumSlots;
		Node* IterNode = m_Table[ Index ];
		while( IterNode )
		{
			if( IterNode->m_Key.Equals( Key ) )
			{
				return Iterator( IterNode );
			}
			IterNode = IterNode->m_Next;
		}
		return Iterator( NULL );
	}

	void Remove( const HashedString& Key )
	{
		uint Index = Key.GetHash() % m_NumSlots;
		Node* Head = m_Table[ Index ];
		Node* IterNode = Head;

		while( IterNode )
		{
			Node* Next = IterNode->m_Next;
			if( IterNode->m_Key.Equals( Key ) )
			{
				// This must be the head
				Node* NewHead = Head->m_Next;
				delete Head;
				m_Table[ Index ] = NewHead;
				--m_Size;
				return;
			}
			// Check if the *next* item is equal to Item, so we can update the pointers
			else if( Next && Next->m_Key.Equals( Key ) )
			{
				IterNode->m_Next = Next->m_Next;
				delete Next;
				Next = IterNode->m_Next;
				--m_Size;
				return;
			}
			IterNode = Next;
		}
	}

	void Clear()
	{
		Node* ToDelete;
		Node* Next;
		for( uint i = 0; i < m_NumSlots; ++i )
		{
			for( ToDelete = m_Table[i]; ToDelete; ToDelete = Next )
			{
				Next = ToDelete->m_Next;
				delete ToDelete;
			}
			m_Table[i] = NULL;
		}
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

	uint GetNumSlots() const
	{
		return m_NumSlots;
	}

	// Expose the array to allow iterating over every item in table
	Iterator Begin( uint Slot ) const
	{
		return Iterator( m_Table[ Slot ] );
	}

	Iterator End() const
	{
		return Iterator( NULL );
	}

private:
	Node**	AllocateNodeArray()
	{
		if( m_Allocator )
		{
			return new( *m_Allocator ) Node*[ m_NumSlots ];
		}
		else
		{
			return new Node*[ m_NumSlots ];
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
};

#endif // HASHTABLE_H