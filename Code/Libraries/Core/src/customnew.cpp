#include "core.h"
#include "customnew.h"
#include "allocator.h"
#include "allocatorchunk.h"

#include <stdlib.h>

void* operator new( const size_t Size, Allocator& Alloc )
{
	return Alloc.Allocate( static_cast<uint>( Size ) );
}

void* operator new( const size_t Size, Allocator& Alloc, const uint Alignment )
{
	return Alloc.Allocate( static_cast<uint>( Size ), Alignment );
}

void* operator new[]( const size_t Size, Allocator& Alloc )
{
	return Alloc.Allocate( static_cast<uint>( Size ) );
}

void* operator new[]( const size_t Size, Allocator& Alloc, const uint Alignment )
{
	return Alloc.Allocate( static_cast<uint>( Size ), Alignment );
}

// These forms aren't actually used.
void operator delete( void* pObj, Allocator& Alloc )
{
	Unused( pObj );
	Unused( Alloc );

	DEBUGBREAKPOINT;
}

void operator delete( void* pObj, Allocator& Alloc, const uint Alignment )
{
	Unused( pObj );
	Unused( Alloc );
	Unused( Alignment );

	DEBUGBREAKPOINT;
}

void operator delete[]( void* pObj, Allocator& Alloc )
{
	Unused( pObj );
	Unused( Alloc );

	DEBUGBREAKPOINT;
}

void operator delete[]( void* pObj, Allocator& Alloc, const uint Alignment )
{
	Unused( pObj );
	Unused( Alloc );
	Unused( Alignment );

	DEBUGBREAKPOINT;
}

#if USE_ALLOCATOR
void* operator new( const size_t Size ) NEW_THROW
{
	void* pObj = NULL;
	if( Allocator::IsEnabled() )
	{
		pObj = Allocator::GetDefault().Allocate( static_cast<uint>( Size ) );
	}
	else
	{
		pObj = malloc( Size );
	}
	DEVASSERT( pObj );
	return pObj;
}

void* operator new( const size_t Size, uint Alignment )
{
	return Allocator::GetDefault().Allocate( static_cast<uint>( Size ), Alignment );
}

void* operator new[]( const size_t Size ) NEW_THROW
{
	void* pObj = NULL;
	if( Allocator::IsEnabled() )
	{
		pObj = Allocator::GetDefault().Allocate( static_cast<uint>( Size ) );
	}
	else
	{
		pObj = malloc( Size );
	}
	DEVASSERT( pObj );
	return pObj;
}

void* operator new[]( const size_t Size, uint Alignment )
{
	return Allocator::GetDefault().Allocate( static_cast<uint>( Size ), Alignment );
}

void operator delete( void* pObj ) DELETE_THROW
{
	if( !pObj )
	{
		return;
	}

	if( Allocator::IsEnabled() )
	{
		AllocatorChunk* const pChunk = *( AllocatorChunk** )( static_cast<byte*>( pObj ) - Allocator::m_BlockHeaderSize );
		DEVASSERT( pChunk );
		pChunk->Free( pObj );
	}
	else
	{
		free( pObj );
	}
}

void operator delete( void* pObj, uint Alignment )
{
	Unused( Alignment );

	if( !pObj )
	{
		return;
	}

	// No need to check if Allocator::IsEnabled(), if this form is used then it should be
	AllocatorChunk* const pChunk = *( AllocatorChunk** )( static_cast<byte*>( pObj ) - Allocator::m_BlockHeaderSize );
	DEVASSERT( pChunk );
	pChunk->Free( pObj );
}

void operator delete[]( void* pObj ) DELETE_THROW
{
	if( !pObj )
	{
		return;
	}

	if( Allocator::IsEnabled() )
	{
		AllocatorChunk* const pChunk = *( AllocatorChunk** )( static_cast<byte*>( pObj ) - Allocator::m_BlockHeaderSize );
		DEVASSERT( pChunk );
		pChunk->Free( pObj );
	}
	else
	{
		free( pObj );
	}
}

void operator delete[]( void* pObj, uint Alignment )
{
	Unused( Alignment );

	if( !pObj )
	{
		return;
	}

	// No need to check if Allocator::IsEnabled(), if this form is used then it should be
	AllocatorChunk* const pChunk = *( AllocatorChunk** )( static_cast<byte*>( pObj ) - Allocator::m_BlockHeaderSize );
	DEVASSERT( pChunk );
	pChunk->Free( pObj );
}
#endif // USE_ALLOCATOR