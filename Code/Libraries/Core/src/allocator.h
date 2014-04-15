#ifndef ALLOCATOR_H
#define ALLOCATOR_H

class IDataStream;
struct SAllocatorChunk;

class Allocator
{
public:
	Allocator( const char* const AllocatorName );
	~Allocator();

	static Allocator&	GetDefault();
	static void			Enable( const bool Enable );
	static bool			IsEnabled();

	// Safe to use when Allocator is not meant to be enabled
	static Allocator*	GetDefaultOverride();										// Just intended for use with SScopedAllocator
	static void			SetDefaultOverride( Allocator* const pDefaultOverride );	// Call with NULL to revert to actual default

	struct SChunkDef
	{
		SChunkDef()
		:	m_AllocSize( 0 )
		,	m_ChunkSize( 0 )
		{
		}

		uint	m_AllocSize;	// Allocations this size or smaller may be made in this chunk (if 0, it is unbounded)
		uint	m_ChunkSize;	// The chunk is this large (for initialization)
	};

	void	Initialize( const uint Size );
	void	Initialize( const uint NumChunks, const SChunkDef* const pChunkDefs );
	void	ShutDown();

	void*	Allocate( const uint Size, const uint Alignment = m_Granularity );
	void	Flush();

#if BUILD_DEV
	uint	IncrementNumAllocs() { return m_NumAllocs++; }
#endif

	uint	GetBookmark() const;	// For leak checking
	void	Report( const IDataStream& Stream ) const;
	void	ReportMemoryLeaks( const IDataStream& Stream, const uint Bookmark1, const uint Bookmark2 ) const;

	// Simple test--return true if no leaks
	bool	CheckForLeaks() const;
	bool	CheckForLeaks( const uint Bookmark1, const uint Bookmark2 ) const;

	// Debug stats, return 0 in other builds
	void	GetStats(
		uint* const pAllocated,
		uint* const pOverhead,
		uint* const pMaxOverhead,
		uint* const pFree,
		uint* const pMinFree,
		uint* const pMaxAllocated,
		uint* const pNumBlocks,
		uint* const pMaxBlocks ) const;

	static const uint	m_Granularity;
	static const uint	m_GranularityMask;
	static const uint	m_BlockSize;
	static const uint	m_BlockHeaderSize;

private:
	uint				m_NumChunks;
	SAllocatorChunk*	m_Chunks;

#if BUILD_DEV
	uint				m_NumAllocs;	// For leak checking
	const char*			m_Name;
#endif // BUILD_DEV

	static bool			m_DefaultEnabled;
	static Allocator*	m_DefaultOverride;
};

// Scoped default allocator override. Returns to the
// actual default when this falls out of scope. Safe to use
// even when allocator is not enabled, and safe to nest.
struct SScopedAllocator
{
public:
	SScopedAllocator( Allocator* const pOverride );
	~SScopedAllocator();

private:
	Allocator*	m_PreviousAllocator;
};

#endif // ALLOCATOR_H