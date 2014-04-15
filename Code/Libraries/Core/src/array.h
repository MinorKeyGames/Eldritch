#ifndef ARRAY_H
#define ARRAY_H

#include <memory.h>
#include <stdlib.h>
#include <new>		// Required for BUILD_LINUX, at least

// If defined, array reallocates to half its capacity when size falls below a quarter
// capacity and will be deallocated completely when the array is cleared.
#define DEFLATE

#define FOR_EACH_ARRAY( iter, array, type ) for( Array<type>::Iterator iter = (array).Begin(); iter.IsValid(); ++iter )
#define FOR_EACH_ARRAY_NOINCR( iter, array, type ) for( Array<type>::Iterator iter = (array).Begin(); iter.IsValid(); )
#define FOR_EACH_ARRAY_REVERSE( iter, array, type ) for( Array<type>::Iterator iter = (array).End(); iter.IsValid(); --iter )
#define FOR_EACH_ARRAY_REVERSE_NOINCR( iter, array, type ) for( Array<type>::Iterator iter = (array).End(); iter.IsValid(); )

// This doesn't really belong here, but whatever.
#define FOR_EACH_INDEX( idx, num ) for( uint idx = 0; idx < num; ++idx )

// Dynamic array (i.e., vector--but not named such to avoid conflict with math vector)

template<class C> class Array
{
public:
	template<class IC> class _Iterator
	{
		friend class Array;

	private:
		_Iterator<IC>()
		:	m_Array( NULL )
		,	m_Index( 0 )
		{
		}

	public:
		_Iterator<IC>( const Array<IC>* const pArray, const uint Index )
		:	m_Array( const_cast<Array<IC>*>( pArray ) )
		,	m_Index( Index )
		{
		}

		_Iterator<IC>( const _Iterator<IC>& Other )
		:	m_Array( Other.m_Array )
		,	m_Index( Other.m_Index )
		{
		}

		bool operator==( const _Iterator<IC>& Other ) const
		{
			return m_Array == Other.m_Array && m_Index == Other.m_Index;
		}

		bool operator!=( const _Iterator<IC>& Other ) const
		{
			return m_Array != Other.m_Array || m_Index != Other.m_Index;
		}

		_Iterator<IC>& operator=( const _Iterator<IC>& Other )
		{
			m_Index = Other.m_Index;
			return *this;
		}

		IC& operator*() const
		{
			return ( *m_Array )[ m_Index ];
		}

		// Same as operator*
		IC& GetValue() const
		{
			return ( *m_Array )[ m_Index ];
		}

		uint GetIndex() const
		{
			return m_Index;
		}

		bool IsValid() const
		{
			return m_Array && m_Index < ( *m_Array ).Size();
		}

		bool IsNull() const
		{
			return !m_Array || m_Index >= ( *m_Array ).Size();
		}

		// Prefix
		_Iterator<IC>& operator++()
		{
			++m_Index;
			return *this;
		}

		_Iterator<IC>& operator--()
		{
			--m_Index;
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

	private:
		Array<IC>*	m_Array;
		uint		m_Index;
	};

	typedef _Iterator<C>	Iterator;

private:
	C*				m_Array;
	uint			m_Size;
	uint			m_Capacity;
#ifdef DEFLATE
	uint			m_Reserved;
	bool			m_Deflate;
#endif
	Allocator*	m_Allocator;

public:
	Array( Allocator* pAllocator = NULL )
		:	m_Array( NULL )
		,	m_Size( 0 )
		,	m_Capacity( 0 )
#ifdef DEFLATE
		,	m_Reserved( 0 )
		,	m_Deflate( true )
#endif
		,	m_Allocator( pAllocator )
	{
	}

	Array( const Array& Other, Allocator* pAllocator = NULL )
		:	m_Array( NULL )
		,	m_Size( 0 )
		,	m_Capacity( 0 )
#ifdef DEFLATE
		,	m_Reserved( 0 )
		,	m_Deflate( Other.m_Deflate )
#endif
		,	m_Allocator( pAllocator )
	{
#ifdef DEFLATE
		Reserve( Other.m_Reserved );
#endif
		uint NewSize = Other.Size();
		if( NewSize )
		{
			ExpandTo( NewSize );
			memcpy( m_Array, Other.m_Array, sizeof( C ) * NewSize );
			m_Size = NewSize;
		}
	}

	~Array()
	{
		Clear();
		Free();
	}

	Array& operator=( const Array& Other )
	{
		Clear();

		// I'm intentionally not setting m_Reserved or m_Deflate here.
		// This array shouldn't necessarily inherit those properties.

		uint NewSize = Other.Size();
		if( NewSize )
		{
			ExpandTo( NewSize );
			memcpy( m_Array, Other.m_Array, sizeof( C ) * NewSize );
			m_Size = NewSize;
		}

		return *this;
	}

	// This now properly resizes downward if DEFLATE is defined
	void	Resize( uint Size )
	{
		if( Size == 0 )
		{
			Clear();
		}
		else
		{
			uint TargetCapacity = 0;
			if( !m_Array || Size > m_Capacity )
			{
				// Grow by doubling, just like when pushing back
				TargetCapacity = m_Capacity > 0 ? m_Capacity : 1;
			}
#ifdef DEFLATE
			else if( m_Deflate && Size <= ( m_Capacity >> 1 ) )
			{
				// Deflate
				TargetCapacity = 1;
			}
#endif

			if( TargetCapacity )
			{
				while( Size > TargetCapacity )
				{
					TargetCapacity <<= 1;
				}
				ExpandTo( TargetCapacity );
			}

			m_Size = Size;
		}
	}

	void	ResizeZero( uint Size )
	{
		Resize( Size );
		MemoryZero();
	}

	void	MemoryZero()
	{
		memset( GetData(), 0, MemorySize() );
	}

	void	Append( const Array& Other )
	{
		if( Other.m_Size > 0 )
		{
			if( m_Capacity < m_Size + Other.m_Size )
			{
				ExpandTo( m_Size + Other.m_Size );
			}
			memcpy( m_Array + m_Size, Other.m_Array, sizeof( C ) * Other.m_Size );
			m_Size += Other.m_Size;
		}
	}

	C&	PushBack( const C& Item )
	{
		const C& TempItem = Item;	// In case Item is in this Array and it expands
		if( !m_Array || m_Size == m_Capacity )
		{
			ExpandTo( m_Capacity << 1 );
		}
		new( &m_Array[ m_Size++ ] ) C( TempItem );	// Construct the object now using the copy constructor--it hasn't been constructed yet

		return Last();
	}

	C&	PushBack()
	{
		return PushBack( C() );
	}

	void	Insert( const C& Item )
	{
		PushBack( Item );
	}

	// Slow, consider using a List or other container
	void	Insert( const C& Item, uint Index )
	{
		if( Index == m_Size )
		{
			PushBack( Item );
		}
		else if( Index < m_Size )
		{
			const C& TempItem = Item;	// In case Item is in this Array and it expands

			if( !m_Array || m_Size == m_Capacity )
			{
				ExpandTo( m_Capacity << 1 );
			}

			m_Size++;

			// Copy elements forward (one at a time instead of as a whole block;
			// seems slow, but moving a whole block requires allocating a temp
			// buffer for the move, which could be bad for very large arrays).
			uint Count = m_Size - Index - 1;
			for( uint c = 0, i = m_Size - 1; c < Count; --i, ++c )	// Doing this madness to avoid wrapping the uint
			{
				memcpy( m_Array + i, m_Array + i - 1, sizeof( C ) );
			}

			new( &m_Array[ Index ] ) C( TempItem );	// Construct the object now using the copy constructor--it hasn't been constructed yet
		}
	}

	// Slow, consider using a Set or other container
	bool	Find( const C& Item, uint* const pOutIndex = NULL ) const
	{
		for( uint i = 0; i < m_Size; ++i )
		{
			if( m_Array[i] == Item )
			{
				if( pOutIndex )
				{
					*pOutIndex = i;
				}
				return true;
			}
		}

		return false;
	}

	bool	Find( const C& Item, uint& OutIndex ) const
	{
		return Find( Item, &OutIndex );
	}

	// Requires C::operator== to be implemented
	void	PushBackUnique( const C& Item )
	{
		uint Index;
		if( !Find( Item, Index ) )
		{
			PushBack( Item );
		}
	}

	void	PopBack()
	{
		if( m_Size > 0 )
		{
			m_Array[ --m_Size ].~C();
#ifdef DEFLATE
			// In some cases (e.g., if some amount was reserved and then unreserved),
			// then m_Capacity >> 1 isn't actually the smallest power of two that
			// will contain m_Size. But that's a contrived case and doesn't cause
			// problems, so whatever.
			if( m_Deflate && m_Size <= ( m_Capacity >> 2 ) )
			{
				ExpandTo( m_Capacity >> 1 );
			}
#endif
		}
	}

	// Slow! Prefer to use linked lists (or FastRemove) when possible
	void	Remove( uint Index )
	{
		if( m_Size > Index )
		{
			m_Array[ Index ].~C();

			// Copy elements back (one at a time instead of as a whole block;
			// seems slow, but moving a whole block requires allocating a temp
			// buffer for the move, which could be bad for very large arrays).
			for( uint i = Index; i < m_Size - 1; ++i )
			{
				memcpy( m_Array + i, m_Array + i + 1, sizeof( C ) );
			}

			--m_Size;
#ifdef DEFLATE
			// In some cases (e.g., if some amount was reserved and then unreserved),
			// then m_Capacity >> 1 isn't actually the smallest power of two that
			// will contain m_Size. But that's a contrived case and doesn't cause
			// problems, so whatever.
			if( m_Deflate && m_Size <= ( m_Capacity >> 2 ) )
			{
				ExpandTo( m_Capacity >> 1 );
			}
#endif
		}
	}

	void	Remove( const uint Index, uint Num )
	{
		DEVASSERT( Index < m_Size );
		// Num = Min( Num, m_Size - Index );
		Num = ( Num < ( m_Size - Index ) ) ? Num : ( m_Size - Index );

		if( Num == 0 )
		{
			return;
		}

		const uint DestructEndIndex = Index + Num;
		for( uint DestructIndex = Index; DestructIndex < DestructEndIndex; ++DestructIndex )
		{
			m_Array[ DestructIndex ].~C();
		}

		// Copy elements back (one at a time instead of as a whole block;
		// seems slow, but moving a whole block requires allocating a temp
		// buffer for the move, which could be bad for very large arrays).
		const uint CopyEndIndex = m_Size - Num;
		for( uint CopyIndex = Index; CopyIndex < CopyEndIndex; ++CopyIndex )
		{
			memcpy( m_Array + CopyIndex, m_Array + CopyIndex + Num, sizeof( C ) );
		}

		m_Size -= Num;

#ifdef DEFLATE
		if( m_Deflate )
		{
			if( m_Size == 0 )
			{
				Free();
			}
			// TODO: Deflate properly for the number removed.
			else if( m_Size <= ( m_Capacity >> 2 ) )
			{
				ExpandTo( m_Capacity >> 1 );
			}
		}
#endif
	}

	// Slightly faster for large arrays (one small memcpy); swaps last
	// element into the placed of the removed item, so it's not stable.
	void	FastRemove( uint Index )
	{
		if( m_Size > Index )
		{
			m_Array[ Index ].~C();

			// Copy last element into place
			if( Index < m_Size - 1 )
			{
				memcpy( m_Array + Index, m_Array + m_Size - 1, sizeof( C ) );
			}
			
			--m_Size;
#ifdef DEFLATE
			// In some cases (e.g., if some amount was reserved and then unreserved),
			// then m_Capacity >> 1 isn't actually the smallest power of two that
			// will contain m_Size. But that's a contrived case and doesn't cause
			// problems, so whatever.
			if( m_Deflate )
			{
				if( m_Size == 0 )
				{
					Free();
				}
				else if( m_Size <= ( m_Capacity >> 2 ) )
				{
					ExpandTo( m_Capacity >> 1 );
				}
			}
#endif
		}
	}

	// Slow! Prefer to use linked lists (or FastRemoveItem) when possible
	void	RemoveItem( const C& Item )
	{
		uint Index;
		if( Find( Item, Index ) )
		{
			Remove( Index );
		}
	}

	void	FastRemoveItem( const C& Item )
	{
		uint Index;
		if( Find( Item, Index ) )
		{
			FastRemove( Index );
		}
	}

	void	Clear()
	{
		for( uint i = 0; i < m_Size; ++i )
		{
			m_Array[i].~C();
		}

		m_Size = 0;
#ifdef DEFLATE
		if( m_Deflate )
		{
			if( m_Reserved )
			{
				ExpandTo( m_Reserved );
			}
			else
			{
				Free();
			}
		}
#endif
	}

	uint	Size() const
	{
		return m_Size;
	}

	uint	MemorySize() const
	{
		return m_Size * sizeof( C );
	}

	// Only meant for validation that array is working properly;
	// shouldn't ever be needed for any actual use.
	uint	CheckCapacity() const
	{
		return m_Capacity;
	}

	bool	Empty() const
	{
		return m_Size == 0;
	}

	void	Reserve( uint Capacity )
	{
#ifdef DEFLATE
		m_Reserved = Capacity;
#endif
		if( m_Capacity < Capacity )
		{
			ExpandTo( Capacity );
		}
	}

	void	SetDeflate( const bool Deflate )
	{
#ifdef DEFLATE
		m_Deflate = Deflate;
#endif
	}

	C&		First()
	{
		DEVASSERT( m_Size > 0 );
		return m_Array[0];
	}

	C&		Last()
	{
		DEVASSERT( m_Size > 0 );
		return m_Array[ m_Size - 1 ];
	}

	Iterator	Begin() const
	{
		return Iterator( this, 0 );
	}

	Iterator	End() const
	{
		return Iterator( this, m_Size - 1 );
	}

	Iterator	Search( const C& Item ) const
	{
		uint SearchIndex;
		if( Find( Item, SearchIndex ) )
		{
			return Iterator( this, SearchIndex );
		}
		else
		{
			return Iterator();
		}
	}

	void		Remove( const Iterator& Iter )
	{
		Remove( Iter.m_Index );
	}

	void		FastRemove( const Iterator& Iter )
	{
		FastRemove( Iter.m_Index );
	}

	C&		operator[]( uint Index )
	{
		DEVASSERTDESC( Index < m_Size, "Array index out of bounds." );
		return m_Array[ Index ];
	}

	const C&	operator[]( uint Index ) const
	{
		DEVASSERTDESC( Index < m_Size, "Array index out of bounds." );
		return m_Array[ Index ];
	}

	// Return base address of array
	// Not safe to store this value in case array is resized, but
	// avoids the hack of getting the address of the first element.
	C*		GetData() const
	{
		if( m_Size )
		{
			return m_Array;
		}
		else
		{
			return NULL;
		}
	}

	// Provided for convenience, but if frequent sorting is needed, use a different structure.
	// Requires < operator to be implemented for type C.
	// Quicksort is O(n log n) and requires O(n) space. (Use insertion sort if allocation cost is a problem.)
	void QuickSort()
	{
		if( m_Size > 1 )
		{
			const uint PivotIndex = m_Size / 2;
			Array<C> Less;
			Array<C> Greater;

			const C PivotValue = m_Array[ PivotIndex ];

			for( uint Index = 0; Index < m_Size; ++Index )
			{
				if( Index != PivotIndex )
				{
					const C& IterValue = m_Array[ Index ];
					if( IterValue < PivotValue )
					{
						Less.PushBack( IterValue );
					}
					else
					{
						Greater.PushBack( IterValue );
					}
				}
			}

			Less.QuickSort();
			Greater.QuickSort();

			const uint LessSize = Less.Size();
			const uint GreaterSize = Greater.Size();

			DEVASSERT( m_Size == LessSize + GreaterSize + 1 );

			memcpy( m_Array, Less.m_Array, sizeof( C ) * LessSize );
			memcpy( m_Array + LessSize, &PivotValue, sizeof( C ) );
			memcpy( m_Array + LessSize + 1, Greater.m_Array, sizeof( C ) * GreaterSize );
		}
	}

	// Insertion sort is O(n^2) and requires O(1) space.
	void InsertionSort()
	{
		for( uint Index = 0; Index < m_Size; ++Index )
		{
			// Create a copy of the item. It's possible I don't want to do this in some cases.
			// A single copy of the whole array upfront might be more cost effective, but this
			// does have the benefit of not hitting the heap.
			const C IterValue = m_Array[ Index ];

			uint Hole = Index;
			while( Hole > 0 && IterValue < m_Array[ Hole - 1 ] )
			{
				m_Array[ Hole ] = m_Array[ Hole - 1 ];
				Hole = Hole - 1;
			}

			m_Array[ Hole ] = IterValue;
		}
	}

private:
	void	ExpandTo( uint Capacity )
	{
#ifdef DEFLATE
		// Never deflate to less than reserved
		if( Capacity < m_Reserved )
		{
			Capacity = m_Reserved;
		}
#endif

		if( Capacity < 1 )
		{
			Capacity = 1;
		}

		if( Capacity == m_Capacity )
		{
			return;
		}

		// Allocate new array (use byte, not C, because I'm *not* constructing these)
		C* const NewArray = Allocate( Capacity );
		DEVASSERT( NewArray );

		if( m_Array )
		{
			uint SizeToCopy = sizeof( C ) * ( ( m_Size < Capacity ) ? m_Size : Capacity );
			memcpy( NewArray, m_Array, SizeToCopy );
			Free();
		}

		m_Array		= NewArray;
		m_Capacity	= Capacity;
	}

private:
	C* Allocate( uint Size )
	{
		if( m_Allocator )
		{
			return reinterpret_cast<C*>( new( *m_Allocator ) byte[ sizeof( C ) * Size ] );
		}
		else
		{
			return reinterpret_cast<C*>( new byte[ sizeof( C ) * Size ] );
		}
	}

	void Free()
	{
		if( m_Array )
		{
			// Free array (use byte, not C, because I'm not destructing these)
			delete[] reinterpret_cast< byte* >( m_Array );
			m_Array = NULL;
			m_Capacity = 0;
		}
	}
};

#endif // ARRAY_H