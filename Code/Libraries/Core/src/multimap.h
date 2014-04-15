#ifndef MULTIMAP_H
#define MULTIMAP_H

#include "map.h"
#include "list.h"

// Multimap (multiply associative array) implemented with a Map of Lists.
// The Key class must implement < and == to compile this.
// The Value class must implement == to compile this.

#define FOR_EACH_MULTIMAP( iter, map, ktype, vtype ) for( Multimap<ktype, vtype>::Iterator iter = (map).Begin(); iter != (map).End(); ++iter )
#define FOR_EACH_MULTIMAP_NOINCR( iter, map, ktype, vtype ) for( Multimap<ktype, vtype>::Iterator iter = (map).Begin(); iter != (map).End(); )
#define FOR_EACH_MULTIMAP_REVERSE( iter, map, ktype, vtype ) for( Multimap<ktype, vtype>::Iterator iter = (map).Last(); iter != (map).End(); --iter )
#define FOR_EACH_MULTIMAP_REVERSE_NOINCR( iter, map, ktype, vtype ) for( Multimap<ktype, vtype>::Iterator iter = (map).Last(); iter != (map).End(); )
#define FOR_EACH_MULTIMAP_SEARCH( iter, map, ktype, vtype, key ) const ktype& AutoMultimapSearchKey = key; for( Multimap<ktype, vtype>::Iterator iter = (map).Search( AutoMultimapSearchKey ); !iter.IsNull(); iter = (map).Search( AutoMultimapSearchKey, iter ) )

template<class K, class V> class Multimap
{
public:
	typedef typename Map<K, List<V> >::Iterator MapIterator;
	typedef typename List<V>::Iterator ListIterator;

	template<class IK, class IV> class _Iterator
	{
		friend class Multimap;

	public:
		_Iterator<IK, IV>() : m_MapIter(), m_ListIter() {}
		_Iterator<IK, IV>( MapIterator MapIter, ListIterator ListIter ) : m_MapIter( MapIter ), m_ListIter( ListIter ) {}
		_Iterator<IK, IV>( const _Iterator<IK, IV>& Other ) : m_MapIter( Other.m_MapIter ), m_ListIter( Other.m_ListIter ) {}

		bool operator==( const _Iterator<IK, IV>& Other ) const
		{
			// No need to check m_MapIter; it would be redundant
			return m_ListIter == Other.m_ListIter;
		}

		bool operator!=( const _Iterator<IK, IV>& Other ) const
		{
			// No need to check m_MapIter; it would be redundant
			return m_ListIter != Other.m_ListIter;
		}

		_Iterator<IK, IV>& operator=( const _Iterator<IK, IV>& Other )
		{
			m_MapIter = Other.m_MapIter;
			m_ListIter = Other.m_ListIter;
			return *this;
		}

		IV& operator*() const
		{
			return *m_ListIter;
		}

		// Useful when iterating the map to get back to the key
		IK& GetKey() const
		{
			return m_MapIter.GetKey();
		}

		// Same as operator*
		IV& GetValue() const
		{
			return ( *m_ListIter );
		}

		// Prefix
		_Iterator<IK, IV>& operator++()
		{
			DEVASSERT( !m_MapIter.IsNull() && !m_ListIter.IsNull() );
			if( !m_ListIter.IsNull() )
			{
				++m_ListIter;
				if( m_ListIter.IsNull() )
				{
					++m_MapIter;
					if( !m_MapIter.IsNull() )
					{
						m_ListIter = ( *m_MapIter ).Front();
					}
				}
			}
			return *this;
		}

		_Iterator<IK, IV>& operator--()
		{
			DEVASSERT( !m_MapIter.IsNull() && !m_ListIter.IsNull() );
			if( !m_ListIter.IsNull() )
			{
				--m_ListIter;
				if( m_ListIter.IsNull() )
				{
					--m_MapIter;
					if( !m_MapIter.IsNull() )
					{
						m_ListIter = ( *m_MapIter ).Back();
					}
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
			// No need to check m_MapIter; it would be redundant (unless something's broken)
			return m_ListIter.IsNull();
		}

		bool IsValid() const
		{
			return m_ListIter.IsValid();
		}

	private:
		MapIterator		m_MapIter;
		ListIterator	m_ListIter;
	};

	typedef _Iterator<K, V>	Iterator;

private:
	Map<K, List<V> >	m_Map;
	uint			m_Size;

public:
	Multimap( Allocator* pAllocator = NULL )
		:	m_Map( pAllocator )
		,	m_Size( 0 )
	{
	}

	Multimap( const Multimap& Other, Allocator* pAllocator = NULL )
		:	m_Map( pAllocator )
		,	m_Size( 0 )
	{
		for( Multimap<K, V>::Iterator OtherIterator = Other.Begin(); OtherIterator != Other.End(); ++OtherIterator )
		{
			Insert( OtherIterator.GetKey(), OtherIterator.GetValue() );
		}
	}

	~Multimap()
	{
		Clear();
	};

	Multimap& operator=( const Multimap& Other )
	{
		Clear();
		for( Multimap<K, V>::Iterator OtherIterator = Other.Begin(); OtherIterator != Other.End(); ++OtherIterator )
		{
			Insert( OtherIterator.GetKey(), OtherIterator.GetValue() );
		}
		return *this;
	}

	// Insert the item, creating a list in the map if needed.
	// (Not using map's operator[] because it doesn't return an iterator)
	Iterator Insert( const K& Key, const V& Value )
	{
		MapIterator MapIter = m_Map.Search( Key );
		if( MapIter.IsNull() )
		{
			MapIter = m_Map.Insert( Key, List<V>() );
		}
		( *MapIter ).PushBack( Value );
		ListIterator ListIter = ( *MapIter ).Back();
		++m_Size;
		return Iterator( MapIter, ListIter );
	}

	// Returns the first node in the list that matches the key, if any
	Iterator Search( const K& Key ) const
	{
		MapIterator MapIter = m_Map.Search( Key );
		if( MapIter.IsNull() )
		{
			return Iterator( MapIterator(), ListIterator() );
		}
		else
		{
			return Iterator( MapIter, ( *MapIter ).Begin() );
		}
	}

	// Return the next node that matches the key; if the incoming iterator is null, restart from the head.
	Iterator Search( const K& Key, Iterator& Prev ) const
	{
		if( Prev.IsNull() )
		{
			return Search( Key );
		}
		else
		{
			DEBUGASSERT( Prev.m_MapIter == m_Map.Search( Key ) );
			return Iterator( Prev.m_MapIter, ++Prev.m_ListIter );
		}
	}

	// Return the node that matches the key and value (slightly faster than just searching Value)
	Iterator Search( const K& Key, const V& Value ) const
	{
		MapIterator MapIter = m_Map.Search( Key );
		if( !MapIter.IsNull() )
		{
			ListIterator ListIter = ( *MapIter ).Begin();
			ListIterator IterEnd = ( *MapIter ).End();
			for( ; ListIter != IterEnd; ++ListIter )
			{
				if( ( *ListIter ) == Value )
				{
					return Iterator( MapIter, ListIter );
				}
			}
		}
		return Iterator( MapIterator(), ListIterator() );
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
		return Iterator( MapIterator(), ListIterator() );
	}

	// Remove the first entry with this key
	void Remove( const K& Key )
	{
		Iterator Iter( Search( Key ) );
		if( !Iter.IsNull() )
		{
			Remove( Iter );
		}
	}

	void Remove( const K& Key, const V& Value )
	{
		Iterator Iter( Search( Key, Value ) );
		if( !Iter.IsNull() )
		{
			Remove( Iter );
		}
	}

	void RemoveValue( const V& Value )
	{
		Iterator Iter( SearchValue( Value ) );
		if( !Iter.IsNull() )
		{
			Remove( Iter );
		}
	}

	void Remove( Iterator& Iter )
	{
		( *Iter.m_MapIter ).Pop( Iter.m_ListIter );
		if( ( *Iter.m_MapIter ).Empty() )
		{
			// Remove the list when it becomes empty
			m_Map.Remove( Iter.m_MapIter );
		}
		--m_Size;
	}

	// Remove all entries with this key
	void RemoveAll( const K& Key )
	{
		MapIterator MapIter( m_Map.Search( Key ) );
		if( !MapIter.IsNull() )
		{
			uint SizeRemoved = ( *MapIter ).Size();
			m_Map.Remove( MapIter );
			m_Size -= SizeRemoved;
		}
	}

	void Clear()
	{
		m_Map.Clear();
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

	// NOTE: Side effect: creates the entry if the key is not found
	// This syntax isn't really as useful for a Multimap as a map, but why not support it?
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
	const V& operator[]( const K& Key ) const
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
		MapIterator MapIter = m_Map.First();
		if( MapIter.IsNull() )
		{
			return Iterator( MapIterator(), ListIterator() );
		}
		else
		{
			return Iterator( MapIter, ( *MapIter ).Front() );
		}
	}

	// Return last item for inorder traversal
	Iterator Last() const
	{
		MapIterator MapIter = m_Map.Last();
		if( MapIter.IsNull() )
		{
			return Iterator( MapIterator(), ListIterator() );
		}
		else
		{
			return Iterator( MapIter, ( *MapIter ).Back() );
		}
	}

	Iterator End() const
	{
		return Iterator( MapIterator(), ListIterator() );
	}
};

#endif // MULTIMAP_H