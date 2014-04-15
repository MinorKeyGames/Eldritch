#ifndef ALLOCATORCHUNK_H
#define ALLOCATORCHUNK_H

class Allocator;
class IDataStream;

class AllocatorChunk
{
public:
	AllocatorChunk();
	~AllocatorChunk();

	void	Initialize( Allocator* const pAllocator, uint Size );
	void	ShutDown();

	void*	Allocate( uint Size, uint Alignment );
	void	Free( void* const pObj );
	void	Flush();

	void	Report( const IDataStream& Stream ) const;
	void	ReportMemoryLeaks( const IDataStream& Stream, const uint Bookmark1, const uint Bookmark2 ) const;
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

	// Blocks form a doubly-linked list.
	struct SAllocatorBlock
	{
		SAllocatorBlock();

		AllocatorChunk*		m_Chunk;
		bool				m_Used;
		uint				m_Size;
		SAllocatorBlock*	m_Prev;
		SAllocatorBlock*	m_Next;

#if BUILD_DEV
		uint				m_AllocNum;	// For leak checking
#endif // BUILD_DEV
	};

private:
	void	Split( SAllocatorBlock* const BlockToSplit, const uint Size );
	void	SplitToAlignment( SAllocatorBlock* const BlockToSplit, const uint Alignment );
	bool	CanSplitToAlignment( const SAllocatorBlock* const pBlock, const uint Alignment, const uint Size ) const;
	void	Coalesce( SAllocatorBlock* const First );								// Coalesces this block with its next
	bool	IsAligned( const SAllocatorBlock* const pBlock, const uint Alignment ) const;	// True if this block's *data* is aligned

	Allocator*			m_Allocator;
	SAllocatorBlock*	m_Head;
	SAllocatorBlock*	m_Free;	// A known free block for fast allocation. No other guarantees about placement.
	uint				m_Size;

#if BUILD_DEV
	uint				m_AllocSpace;	// Just what is actually allocated by user
	uint				m_Overhead;		// Allocated for use internally
	uint				m_FreeSpace;	// Actual free space remaining (capacity minus allocated and overhead)
	uint				m_NumBlocks;
	uint				m_MaxAlloc;
	uint				m_MaxOverhead;
	uint				m_MinFree;
	uint				m_MaxBlocks;
#endif // BUILD_DEV
};

#endif // ALLOCATORCHUNK_H