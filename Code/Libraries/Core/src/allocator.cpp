#include "core.h"
#include "allocator.h"
#include "allocatorchunk.h"
#include "idatastream.h"

#include <stdlib.h>
#include <new>	// Required for BUILD_LINUX, at least

const uint	Allocator::m_Granularity		= 8;	// Must be power of two
const uint	Allocator::m_GranularityMask	= Allocator::m_Granularity - 1;
const uint	Allocator::m_BlockSize			= sizeof( AllocatorChunk::SAllocatorBlock );
const uint	Allocator::m_BlockHeaderSize	= ( m_BlockSize + Allocator::m_GranularityMask ) & ~Allocator::m_GranularityMask;

bool		Allocator::m_DefaultEnabled		= false;
Allocator*	Allocator::m_DefaultOverride	= NULL;

SScopedAllocator::SScopedAllocator( Allocator* const pOverride )
:	m_PreviousAllocator( Allocator::GetDefaultOverride() )
{
	Allocator::SetDefaultOverride( pOverride );
}

SScopedAllocator::~SScopedAllocator()
{
	Allocator::SetDefaultOverride( m_PreviousAllocator );
}

struct SAllocatorChunk
{
	SAllocatorChunk()
		:	m_AllocSize( 0 )
		,	m_Chunk()
	{
	}

	uint			m_AllocSize;
	AllocatorChunk	m_Chunk;
};

Allocator::Allocator( const char* const AllocatorName )
:	m_NumChunks( 0 )
,	m_Chunks( NULL )
#if BUILD_DEV
,	m_NumAllocs( 0 )
,	m_Name( AllocatorName ? AllocatorName : "" )
#endif // BUILD_DEV
{
	Unused( AllocatorName );
}

Allocator::~Allocator()
{
}

/*static*/ Allocator& Allocator::GetDefault()
{
	static Allocator DefaultAllocator( "Default" );
	return m_DefaultOverride ? *m_DefaultOverride : DefaultAllocator;
}

/*static*/ Allocator* Allocator::GetDefaultOverride()
{
	return m_DefaultOverride;
}

/*static*/ void Allocator::SetDefaultOverride( Allocator* pDefaultOverride )
{
	m_DefaultOverride = pDefaultOverride;
}

/*static*/ void Allocator::Enable( bool Enable )
{
	m_DefaultEnabled = Enable;
}

/*static*/ bool Allocator::IsEnabled()
{
	return m_DefaultEnabled;
}

void Allocator::Initialize( const uint Size )
{
	SChunkDef DefaultChunkDef;
	DefaultChunkDef.m_AllocSize = 0;
	DefaultChunkDef.m_ChunkSize = Size;
	Initialize( 1, &DefaultChunkDef );
}

void Allocator::Initialize( const uint NumChunks, const SChunkDef* const pChunkDefs )
{
	DEVASSERT( !m_Chunks );

	m_NumChunks	= NumChunks;
	m_Chunks	= static_cast<SAllocatorChunk*>( malloc( NumChunks * sizeof( SAllocatorChunk ) ) );

	for( uint ChunkIndex = 0; ChunkIndex < m_NumChunks; ++ChunkIndex )
	{
		const SChunkDef&	ChunkDef = pChunkDefs[ ChunkIndex ];
		SAllocatorChunk&	Chunk = m_Chunks[ ChunkIndex ];

		new(&Chunk) SAllocatorChunk();
		Chunk.m_AllocSize = ChunkDef.m_AllocSize;
		Chunk.m_Chunk.Initialize( this, ChunkDef.m_ChunkSize );
	}
}

void Allocator::ShutDown()
{
	for( uint ChunkIndex = 0; ChunkIndex < m_NumChunks; ++ChunkIndex )
	{
		SAllocatorChunk& Chunk = m_Chunks[ ChunkIndex ];
		Chunk.m_Chunk.ShutDown();
		Chunk.~SAllocatorChunk();
	}

	free( m_Chunks );
	m_Chunks	= NULL;
	m_NumChunks	= 0;
}

void* Allocator::Allocate( const uint Size, const uint Alignment /*= m_Granularity*/ )
{
	for( uint ChunkIndex = 0; ChunkIndex < m_NumChunks; ++ChunkIndex )
	{
		SAllocatorChunk& Chunk = m_Chunks[ ChunkIndex ];
		if( Size <= Chunk.m_AllocSize || Chunk.m_AllocSize == 0 )
		{
			return Chunk.m_Chunk.Allocate( Size, Alignment );
		}
	}

	// No allocator chunk supported the requested size
	DEBUGBREAKPOINT;

	return NULL;
}

void Allocator::Flush()
{
	for( uint ChunkIndex = 0; ChunkIndex < m_NumChunks; ++ChunkIndex )
	{
		SAllocatorChunk& Chunk = m_Chunks[ ChunkIndex ];
		Chunk.m_Chunk.Flush();
	}
}

uint Allocator::GetBookmark() const
{
#if BUILD_DEV
	return m_NumAllocs;
#else
	return 0;
#endif // BUILD_DEV
}

void Allocator::Report( const IDataStream& Stream ) const
{
	Unused( Stream );

#if BUILD_DEV
	Stream.PrintF( "========================================\n\n" );
	Stream.PrintF( "Allocator (%s) report\n\n", m_Name );
	Stream.PrintF( "Total allocations:\t%d\n", m_NumAllocs );

	for( uint ChunkIndex = 0; ChunkIndex < m_NumChunks; ++ChunkIndex )
	{
		const SAllocatorChunk& Chunk = m_Chunks[ ChunkIndex ];

		Stream.PrintF( "Chunk %d, allocation size %d:\n", ChunkIndex, Chunk.m_AllocSize );
		Chunk.m_Chunk.Report( Stream );
	}

	ReportMemoryLeaks( Stream, 0, m_NumAllocs );
#endif // BUILD_DEV
}

void Allocator::ReportMemoryLeaks( const IDataStream& Stream, const uint Bookmark1, const uint Bookmark2 ) const
{
	Unused( Stream );
	Unused( Bookmark1 );
	Unused( Bookmark2 );

#if BUILD_DEV
	Stream.PrintF( "Memory manager leak report:\n\n" );

	for( uint ChunkIndex = 0; ChunkIndex < m_NumChunks; ++ChunkIndex )
	{
		const SAllocatorChunk& Chunk = m_Chunks[ ChunkIndex ];

		Stream.PrintF( "Chunk %d:\n", ChunkIndex, Chunk.m_AllocSize );
		Chunk.m_Chunk.ReportMemoryLeaks( Stream, Bookmark1, Bookmark2 );
	}

	Stream.PrintF( "\n" );
#endif // BUILD_DEV
}

bool Allocator::CheckForLeaks() const
{
#if BUILD_DEV
	return CheckForLeaks( 0, m_NumAllocs );
#else
	return true;
#endif
}

bool Allocator::CheckForLeaks( const uint Bookmark1, const uint Bookmark2 ) const
{
	Unused( Bookmark1 );
	Unused( Bookmark2 );

#if BUILD_DEV
	for( uint ChunkIndex = 0; ChunkIndex < m_NumChunks; ++ChunkIndex )
	{
		const SAllocatorChunk& Chunk = m_Chunks[ ChunkIndex ];
		if( !Chunk.m_Chunk.CheckForLeaks( Bookmark1, Bookmark2 ) )
		{
			return false;
		}
	}
#endif

	return true;
}

void Allocator::GetStats(
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
	for( uint ChunkIndex = 0; ChunkIndex < m_NumChunks; ++ChunkIndex )
	{
		const SAllocatorChunk& Chunk = m_Chunks[ ChunkIndex ];
		Chunk.m_Chunk.GetStats(
			pAllocated,
			pOverhead,
			pMaxOverhead,
			pFree,
			pMinFree,
			pMaxAllocated,
			pNumBlocks,
			pMaxBlocks );
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