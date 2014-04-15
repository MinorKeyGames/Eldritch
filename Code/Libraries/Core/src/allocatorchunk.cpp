#include "core.h"
#include "allocatorchunk.h"
#include "allocator.h"
#include "idatastream.h"

#include <new>	// Required for BUILD_LINUX, at least

#if BUILD_LINUX
// For uintptr_t
#include <stdint.h>
#endif

#if BUILD_DEV
#if BUILD_DEBUG
#define DEBUG_FILL 1
#else
#define DEBUG_FILL 0
#endif
#include <memory.h>
#endif

#if DEBUG_FILL
#define FILL_INIT		0xa0
#define FILL_BLOCK		0xb0
#define FILL_ALLOC		0xc0
#define FILL_FREE		0xd0
#define FILL_SHUTDOWN	0xe0
#define FILL_FLUSH		0xf0
#endif

AllocatorChunk::SAllocatorBlock::SAllocatorBlock()
:	m_Chunk( NULL )
,	m_Used( false )
,	m_Size( 0 )
,	m_Prev( NULL )
,	m_Next( NULL )
{
}

AllocatorChunk::AllocatorChunk()
:	m_Allocator( NULL )
,	m_Head( NULL )
,	m_Free( NULL )
,	m_Size( 0 )
#if BUILD_DEV
,	m_AllocSpace( 0 )
,	m_Overhead( 0 )
,	m_MaxOverhead( 0 )
,	m_FreeSpace( 0 )
,	m_MinFree( 0 )
,	m_MaxAlloc( 0 )
,	m_NumBlocks( 0 )
,	m_MaxBlocks( 0 )
#endif // BUILD_DEV
{
}

AllocatorChunk::~AllocatorChunk()
{
}

void AllocatorChunk::Initialize( Allocator* const pAllocator, uint Size )
{
	DEVASSERT( !m_Head );
	DEVASSERT( pAllocator );
	DEVASSERT( Size > 0 );

	m_Allocator = pAllocator;

	// Round up to granularity
	Size = ( Size + Allocator::m_GranularityMask ) & ~Allocator::m_GranularityMask;

	m_Size				= Size;
	void* const Where	= malloc( Size );				// NOTE: malloc returns an 8-byte aligned pointer in 32-bit Windows, 16-byte aligned in 64-bit
	m_Head				= new( Where ) SAllocatorBlock;	// NOTE: Could just cast Where to an SAllocatorBlock* and initialize, but placement new doesn't have any overhead and I like constructors
	m_Head->m_Chunk		= this;
	m_Head->m_Size		= m_Size - Allocator::m_BlockHeaderSize;
	m_Free				= m_Head;

#if BUILD_DEV
#if DEBUG_FILL
	memset( reinterpret_cast<byte*>( m_Head ) + Allocator::m_BlockSize, FILL_BLOCK, Allocator::m_BlockHeaderSize - Allocator::m_BlockSize );
	memset( reinterpret_cast<byte*>( m_Head ) + Allocator::m_BlockHeaderSize, FILL_INIT, m_Size - Allocator::m_BlockHeaderSize );
#endif
	m_AllocSpace	= 0;
	m_Overhead		= Allocator::m_BlockHeaderSize;
	m_MaxOverhead	= Allocator::m_BlockHeaderSize;
	m_FreeSpace		= m_Size - Allocator::m_BlockHeaderSize;
	m_MinFree		= m_FreeSpace;
	m_MaxAlloc		= 0;
	m_NumBlocks		= 1;
	m_MaxBlocks		= 1;
#endif // BUILD_DEV
}

void AllocatorChunk::ShutDown()
{
	DEVASSERT( m_Head );

#if BUILD_DEV
#if DEBUG_FILL
	memset( static_cast<void*>( m_Head ), FILL_SHUTDOWN, m_Size );
#endif
#endif

	free( m_Head );

	m_Allocator	= NULL;
	m_Head		= NULL;
	m_Free		= NULL;
	m_Size		= 0;

#if BUILD_DEV
	m_AllocSpace	= 0;
	m_Overhead		= 0;
	m_MaxOverhead	= 0;
	m_FreeSpace		= 0;
	m_MinFree		= 0;
	m_MaxAlloc		= 0;
	m_NumBlocks		= 0;
	m_MaxBlocks		= 0;
#endif // BUILD_DEV
}

void* AllocatorChunk::Allocate( uint Size, uint Alignment )
{
	DEVASSERT( m_Head );

	if( !m_Free )
	{
		DEBUGBREAKPOINT;	// Out of memory!
		return NULL;
	}

	// Round alignment up to granularity
	Alignment = ( Alignment + Allocator::m_GranularityMask ) & ~Allocator::m_GranularityMask;

	// Round size up to alignment
	Size += Alignment - 1;
	Size -= Size % Alignment;

	// Use next-fit strategy for now because it's easy and works
	SAllocatorBlock* Fit = m_Free;
	while( Fit )
	{
		// Use first-fit strategy for now because it's easy and works
		if( !Fit->m_Used && Fit->m_Size >= Size )
		{
			if( IsAligned( Fit, Alignment ) )
			{
				break;
			}
			// See if there's room to split this block for alignment
			else if( CanSplitToAlignment( Fit, Alignment, Size ) )
			{
				SplitToAlignment( Fit, Alignment );
				break;
			}
		}

		// Wrap around if we reach the end
		Fit = Fit->m_Next ? Fit->m_Next : m_Head;

		if( Fit == m_Free )
		{
			DEBUGBREAKPOINT;	// Not enough space for allocation!
			return NULL;
		}
	}

	// Split block if necessary
	if( Fit->m_Size > Size + Allocator::m_BlockHeaderSize )
	{
		Split( Fit, Size );
	}

	Fit->m_Used = true;

	// Increment m_Free to the next available free block.
	SAllocatorBlock* NextFree = Fit;
	do
	{
		// Wrap around if we reach the end
		NextFree = NextFree->m_Next ? NextFree->m_Next : m_Head;
	}
	while( NextFree != Fit && NextFree->m_Used );

	m_Free = ( NextFree == Fit ) ? NULL : NextFree;
	DEVASSERT( m_Free );	// Out of memory; not immediately fatal, but we must free something before we can alloc again

#if BUILD_DEV
#if DEBUG_FILL
	memset( (byte*)Fit + Allocator::m_BlockHeaderSize, FILL_ALLOC, Fit->m_Size );
#endif
	Fit->m_AllocNum	= m_Allocator->IncrementNumAllocs();
	m_AllocSpace	+= Fit->m_Size;
	m_MaxAlloc		= ( m_AllocSpace > m_MaxAlloc ) ? m_AllocSpace : m_MaxAlloc;
	m_FreeSpace		-= Fit->m_Size;
	m_MinFree		= ( m_FreeSpace < m_MinFree ) ? m_FreeSpace : m_MinFree;
#endif // BUILD_DEV

	return ( byte* )Fit + Allocator::m_BlockHeaderSize;	// Skip over the block header and return the address of the data
}

void AllocatorChunk::Free( void* pObj )
{
	DEVASSERT( m_Head );

	SAllocatorBlock* const Freed = reinterpret_cast<SAllocatorBlock*>( reinterpret_cast<byte*>( pObj ) - Allocator::m_BlockHeaderSize );
	Freed->m_Used = false;

	if( !m_Free )
	{
		m_Free = Freed;
	}

#if BUILD_DEV
#if DEBUG_FILL
	memset( reinterpret_cast<byte*>( Freed ) + Allocator::m_BlockHeaderSize, FILL_FREE, Freed->m_Size );
#endif
	m_AllocSpace	-= Freed->m_Size;
	m_FreeSpace		+= Freed->m_Size;
#endif // BUILD_DEV

	// Coalesce blocks if possible
	if( Freed->m_Next && !Freed->m_Next->m_Used )
	{
		Coalesce( Freed );
	}
	if( Freed->m_Prev && !Freed->m_Prev->m_Used )
	{
		Coalesce( Freed->m_Prev );
	}
}

// NOTE: This doesn't delete all the data in memory (because I don't
// know their types), it just clears up the memory to be all free
void AllocatorChunk::Flush()
{
	DEVASSERT( m_Head );

	m_Head->m_Next	= NULL;
	m_Head->m_Size	= m_Size - Allocator::m_BlockHeaderSize;
	m_Free			= m_Head;

#if BUILD_DEV
#if DEBUG_FILL
	memset( reinterpret_cast<byte*>( m_Head ) + Allocator::m_BlockHeaderSize, FILL_FLUSH, m_Size - Allocator::m_BlockHeaderSize );
#endif
	m_AllocSpace		= 0;
	m_Overhead			= Allocator::m_BlockHeaderSize;
	m_MaxOverhead		= Allocator::m_BlockHeaderSize;
	m_FreeSpace			= m_Size - Allocator::m_BlockHeaderSize;
	m_MinFree			= m_FreeSpace;
	m_MaxAlloc			= 0;
	m_NumBlocks			= 1;
	m_MaxBlocks			= 1;
#endif // BUILD_DEV
}

void AllocatorChunk::Split( SAllocatorBlock* const BlockToSplit, uint Size )
{
	DEVASSERT( BlockToSplit->m_Size > Size + Allocator::m_BlockHeaderSize );
	void* const Where				= static_cast<void*>( reinterpret_cast<byte*>( BlockToSplit ) + Allocator::m_BlockHeaderSize + Size );
	SAllocatorBlock* const NewBlock	= new( Where ) SAllocatorBlock;	// NOTE: Could just cast Where to a Block* and initialize, but placement new doesn't have any overhead and I like constructors
	NewBlock->m_Chunk				= this;
	NewBlock->m_Size				= BlockToSplit->m_Size - Size - Allocator::m_BlockHeaderSize;
	NewBlock->m_Prev				= BlockToSplit;
	NewBlock->m_Next				= BlockToSplit->m_Next;
	BlockToSplit->m_Size			= Size;
	BlockToSplit->m_Next			= NewBlock;
	if( NewBlock->m_Next )
	{
		NewBlock->m_Next->m_Prev = NewBlock;
	}

#if BUILD_DEV
#if DEBUG_FILL
	memset( reinterpret_cast<byte*>( NewBlock ) + Allocator::m_BlockSize, FILL_BLOCK, Allocator::m_BlockHeaderSize - Allocator::m_BlockSize );
#endif
	m_Overhead		+= Allocator::m_BlockHeaderSize;
	m_MaxOverhead	= ( m_Overhead > m_MaxOverhead ) ? m_Overhead : m_MaxOverhead;
	m_FreeSpace		-= Allocator::m_BlockHeaderSize;
	m_MinFree		= ( m_FreeSpace < m_MinFree ) ? m_FreeSpace : m_MinFree;
	++m_NumBlocks;
	m_MaxBlocks		= ( m_NumBlocks > m_MaxBlocks ) ? m_NumBlocks : m_MaxBlocks;
#endif // BUILD_DEV
}

void AllocatorChunk::SplitToAlignment( SAllocatorBlock* const BlockToSplit, uint Alignment )
{
	const uint AlignPad				= Alignment - ( reinterpret_cast<uintptr_t>( reinterpret_cast<byte*>( BlockToSplit ) + ( Allocator::m_BlockHeaderSize << 1 ) ) % Alignment );
	DEVASSERT( BlockToSplit->m_Size > AlignPad + Allocator::m_BlockHeaderSize );
	void* const Where				= static_cast<void*>( (byte*)BlockToSplit + Allocator::m_BlockHeaderSize + AlignPad );
	SAllocatorBlock* const NewBlock	= new( Where ) SAllocatorBlock;	// NOTE: Could just cast Where to a Block* and initialize, but placement new doesn't have any overhead and I like constructors
	NewBlock->m_Chunk				= this;
	NewBlock->m_Size				= BlockToSplit->m_Size - AlignPad - Allocator::m_BlockHeaderSize;
	NewBlock->m_Prev				= BlockToSplit;
	NewBlock->m_Next				= BlockToSplit->m_Next;
	BlockToSplit->m_Size			= AlignPad;
	BlockToSplit->m_Next			= NewBlock;
	if( NewBlock->m_Next )
	{
		NewBlock->m_Next->m_Prev	= NewBlock;
	}

#if BUILD_DEV
#if DEBUG_FILL
	memset( (byte*)NewBlock + Allocator::m_BlockSize, FILL_BLOCK, Allocator::m_BlockHeaderSize - Allocator::m_BlockSize );
#endif
	m_Overhead		+= Allocator::m_BlockHeaderSize;
	m_MaxOverhead	= ( m_Overhead > m_MaxOverhead ) ? m_Overhead : m_MaxOverhead;
	m_FreeSpace		-= Allocator::m_BlockHeaderSize;
	m_MinFree		= ( m_FreeSpace < m_MinFree ) ? m_FreeSpace : m_MinFree;
	++m_NumBlocks;
	m_MaxBlocks		= ( m_NumBlocks > m_MaxBlocks ) ? m_NumBlocks : m_MaxBlocks;
#endif // BUILD_DEV
}

// Check if an aligned split can accommodate the given size
bool AllocatorChunk::CanSplitToAlignment( const SAllocatorBlock* const BlockToSplit, uint Alignment, uint Size ) const
{
	uint AlignPad			= Alignment - ( reinterpret_cast<uintptr_t>( reinterpret_cast<const byte*>( BlockToSplit ) + ( Allocator::m_BlockHeaderSize << 1 ) ) % Alignment );
	return( BlockToSplit->m_Size >= AlignPad + Allocator::m_BlockHeaderSize + Size );
}

void AllocatorChunk::Coalesce( SAllocatorBlock* const First )
{
	DEVASSERT( !First->m_Used );
	DEVASSERT( First->m_Next );
	DEVASSERT( !First->m_Next->m_Used );
	const SAllocatorBlock* const Next = First->m_Next;
	First->m_Size += Next->m_Size + Allocator::m_BlockHeaderSize;
	First->m_Next = Next->m_Next;
	if( Next->m_Next )
	{
		Next->m_Next->m_Prev = First;
	}

	if( m_Free == Next )
	{
		m_Free = First;
	}

#if BUILD_DEV
#if DEBUG_FILL
	memset( const_cast<byte*>( reinterpret_cast<const byte*>( Next ) ) + Allocator::m_BlockSize, FILL_FREE, Allocator::m_BlockHeaderSize - Allocator::m_BlockSize );
#endif
	m_Overhead	-= Allocator::m_BlockHeaderSize;
	m_FreeSpace	+= Allocator::m_BlockHeaderSize;
	--m_NumBlocks;
#endif // BUILD_DEV
}

bool AllocatorChunk::IsAligned( const SAllocatorBlock* const pBlock, uint Alignment ) const
{
	return ( reinterpret_cast<uintptr_t>( reinterpret_cast<const byte*>( pBlock ) + Allocator::m_BlockHeaderSize ) % Alignment ) == 0;
}

void AllocatorChunk::Report( const IDataStream& Stream ) const
{
	Unused( Stream );

#if BUILD_DEV
	// TODO: Figure out how to quantify fragmentation (Number of free blocks? Size of free blocks? Divided by what?)

	// Sanity checks, no need to report
	DEBUGASSERTDESC( m_FreeSpace == ( m_Size - ( m_AllocSpace + m_Overhead ) ), "Current memory sanity check" );
	DEBUGASSERTDESC( m_MinFree >= ( m_Size - ( m_MaxAlloc + m_MaxOverhead ) ), "Maximum memory sanity check" );

	const float PercentSize = 100.0f / static_cast<float>( m_Size );
	Stream.PrintF( "\tCapacity:\t\t\t%d\n",				m_Size );
	Stream.PrintF( "\tCurrent allocated:\t%d (%f%%)\n",	m_AllocSpace,	static_cast<float>( m_AllocSpace ) * PercentSize );
	Stream.PrintF( "\tCurrent free:\t\t%d (%f%%)\n",	m_FreeSpace,	static_cast<float>( m_FreeSpace ) * PercentSize );
	Stream.PrintF( "\tCurrent overhead:\t%d (%f%%)\n",	m_Overhead,		static_cast<float>( m_Overhead ) * PercentSize );
	Stream.PrintF( "\tCurrent blocks:\t\t%d\n",			m_NumBlocks );
	Stream.PrintF( "\tMaximum allocated:\t%d (%f%%)\n",	m_MaxAlloc,		static_cast<float>( m_MaxAlloc ) * PercentSize );
	Stream.PrintF( "\tMinimum free:\t\t%d (%f%%)\n",	m_MinFree,		static_cast<float>( m_MinFree ) * PercentSize );
	Stream.PrintF( "\tMaximum overhead:\t%d (%f%%)\n",	m_MaxOverhead,	static_cast<float>( m_MaxOverhead ) * PercentSize );
	Stream.PrintF( "\tMaximum blocks:\t\t%d\n",			m_MaxBlocks );

	Stream.PrintF( "\n\tCurrent block map:\n" );
	for( const SAllocatorBlock* pBlock = m_Head; pBlock; pBlock = pBlock->m_Next )
	{
		Stream.PrintF( "\t%s 0x%08X %d\n", pBlock->m_Used ? "USED" : "FREE", pBlock, pBlock->m_Size );
	}
	Stream.PrintF( "\n" );
#endif // BUILD_DEV
}

void AllocatorChunk::ReportMemoryLeaks( const IDataStream& Stream, const uint Bookmark1, const uint Bookmark2 ) const
{
	Unused( Stream );
	Unused( Bookmark1 );
	Unused( Bookmark2 );

#if BUILD_DEV
	for( const SAllocatorBlock* pBlock = m_Head; pBlock; pBlock = pBlock->m_Next )
	{
		if( pBlock->m_Used && pBlock->m_AllocNum >= Bookmark1 && pBlock->m_AllocNum < Bookmark2 )
		{
			Stream.PrintF( "\t0x%08X %d\n", pBlock, pBlock->m_Size );

			static const uint kMaxSizeToPrint = 16;
			const uint SizeToPrint = ( pBlock->m_Size < kMaxSizeToPrint ) ? pBlock->m_Size : kMaxSizeToPrint;

			Stream.PrintF( "\t" );
			for( uint i = 0; i < SizeToPrint; ++i )
			{
				Stream.PrintF( "%02X", *( reinterpret_cast<const byte*>( pBlock ) + Allocator::m_BlockSize + i ) );
				Stream.PrintF( ( i == ( SizeToPrint - 1 ) ) ? "\n" : " " );
			}
		}
	}
#endif // BUILD_DEV
}

bool AllocatorChunk::CheckForLeaks( const uint Bookmark1, const uint Bookmark2 ) const
{
	Unused( Bookmark1 );
	Unused( Bookmark2 );

#if BUILD_DEV
	for( const SAllocatorBlock* pBlock = m_Head; pBlock; pBlock = pBlock->m_Next )
	{
		if( pBlock->m_Used &&
			pBlock->m_AllocNum >= Bookmark1 &&
			pBlock->m_AllocNum < Bookmark2 )
		{
			return false;
		}
	}
#endif // BUILD_DEV

	return true;
}

void AllocatorChunk::GetStats(
		uint* const pAllocated,
		uint* const pOverhead,
		uint* const pMaxOverhead,
		uint* const pFree,
		uint* const pMinFree,
		uint* const pMaxAllocated,
		uint* const pNumBlocks,
		uint* const pMaxBlocks
	) const
{
#if BUILD_DEV
	if( pAllocated )
	{
		*pAllocated += m_AllocSpace;
	}
	if( pOverhead )
	{
		*pOverhead += m_Overhead;
	}
	if( pMaxOverhead )
	{
		*pMaxOverhead += m_MaxOverhead;
	}
	if( pFree )
	{
		*pFree += m_FreeSpace;
	}
	if( pMinFree )
	{
		*pMinFree += m_MinFree;
	}
	if( pMaxAllocated )
	{
		*pMaxAllocated += m_MaxAlloc;
	}
	if( pNumBlocks )
	{
		*pNumBlocks += m_NumBlocks;
	}
	if( pMaxBlocks )
	{
		*pMaxBlocks += m_MaxBlocks;
	}
#else
	if( pAllocated )
	{
		*pAllocated = 0;
	}
	if( pOverhead )
	{
		*pOverhead = 0;
	}
	if( pMaxOverhead )
	{
		*pMaxOverhead = 0;
	}
	if( pFree )
	{
		*pFree = 0;
	}
	if( pMinFree )
	{
		*pMinFree = 0;
	}
	if( pMaxAllocated )
	{
		*pMaxAllocated = 0;
	}
	if( pNumBlocks )
	{
		*pNumBlocks = 0;
	}
	if( pMaxBlocks )
	{
		*pMaxBlocks = 0;
	}
#endif
}