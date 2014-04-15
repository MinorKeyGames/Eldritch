#ifndef PROFILERTABLE_H
#define PROFILERTABLE_H

#if DO_PROFILING

#include "map.h"
#include "arraystack.h"
#include "hashedstring.h"

class IDataStream;

typedef Map< class HashedString, class ProfilerTableEntry > ProfilerMap;

class ProfilerTableEntry
{
public:
	ProfilerTableEntry();
	~ProfilerTableEntry();

	const char*	m_Name;
	__int64		m_TotalInclusiveTime;
	__int64		m_TotalExclusiveTime;
	__int64		m_MaxInclusiveTime;
	__int64		m_MaxExclusiveTime;
	int			m_NumCalls;
};

class ProfilerTable
{
public:
	ProfilerTable();
	~ProfilerTable();

	void				AddEntry( const char* Name, const HashedString& HashedName, __int64 InclusiveTime, __int64 ExclusiveTime );
	void				Dump( const IDataStream& Stream );
	void				Flush();

	const ProfilerMap&	GetEntries();

private:
	ProfilerMap			m_Entries;

public:	// Public for ease of access in ProfilerHook ctor/dtor
	ArrayStack< __int64 >	m_Overhead;		// Subtracted time due to profiler overhead
	ArrayStack< __int64 >	m_Inclusive;	// Subtracted time due to inclusive profiled time
};

#endif // DO_PROFILING

#endif // PROFILERTABLE_H