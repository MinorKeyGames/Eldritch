#include "core.h"

#if DO_PROFILING

#include "idatastream.h"
#include "clock.h"
#include <memory.h>
#include <stdio.h>
#include <string.h>

ProfilerTableEntry::ProfilerTableEntry()
:	m_Name( NULL )
,	m_TotalInclusiveTime( 0 )
,	m_TotalExclusiveTime( 0 )
,	m_MaxInclusiveTime( 0 )
,	m_MaxExclusiveTime( 0 )
,	m_NumCalls( 0 )
{
}

ProfilerTableEntry::~ProfilerTableEntry()
{
}

ProfilerTable::ProfilerTable()
:	m_Entries()
,	m_Overhead()
,	m_Inclusive()
{
	m_Overhead.Push( 0 );
	m_Inclusive.Push( 0 );
}

ProfilerTable::~ProfilerTable()
{
	m_Overhead.Pop();
	m_Inclusive.Pop();
	m_Entries.Clear();
}

void ProfilerTable::AddEntry( const char* Name, const HashedString& HashedName, __int64 InclusiveTime, __int64 ExclusiveTime )
{
	ProfilerTableEntry&	Entry = m_Entries[ HashedName ];

	// Search for an existing entry matching this function sig and update it
	if( Entry.m_NumCalls )
	{
		Entry.m_NumCalls++;
		Entry.m_TotalInclusiveTime += InclusiveTime;
		Entry.m_TotalExclusiveTime += ExclusiveTime;
		if( InclusiveTime > Entry.m_MaxInclusiveTime )
		{
			Entry.m_MaxInclusiveTime = InclusiveTime;
		}
		if( ExclusiveTime > Entry.m_MaxExclusiveTime )
		{
			Entry.m_MaxExclusiveTime = ExclusiveTime;
		}
	}
	else
	{
		// If we get here, a matching entry wasn't found. Add one.
		Entry.m_Name = Name;
		Entry.m_NumCalls = 1;
		Entry.m_TotalInclusiveTime = InclusiveTime;
		Entry.m_TotalExclusiveTime = ExclusiveTime;
		Entry.m_MaxInclusiveTime = InclusiveTime;
		Entry.m_MaxExclusiveTime = ExclusiveTime;
	}
}

void ProfilerTable::Dump( const IDataStream& Stream )
{
	double ClockResolution = Profiler::GetInstance()->m_Clock->GetResolution();

	// Write header
	Stream.PrintF( "FuncSig\tAvgExcTimeSec\tAvgIncTimeSec\tMaxExcTimeSec\tMaxIncTimeSec\tNumCalls\tTotIncTimeCyc\tTotIncTimeSec\tAvgIncTimeCyc\tMaxIncTimeCyc\tTotExcTimeCyc\tTotExcTimeSec\tAvgExcTimeCyc\tMaxExcTimeCyc\n" );

	ProfilerMap::Iterator Iter = m_Entries.Begin();
	ProfilerMap::Iterator End = m_Entries.End();
	for( ; Iter != End; ++Iter )
	{
		ProfilerTableEntry& Entry = *Iter;

		// Function name
		Stream.Write( (int)strlen( Entry.m_Name ), Entry.m_Name );

		// Average exclusive time in seconds
		Stream.PrintF( "\t%f", ( (double)Entry.m_TotalExclusiveTime / (double)Entry.m_NumCalls ) * ClockResolution );

		// Average inclusive time in seconds
		Stream.PrintF( "\t%f", ( (double)Entry.m_TotalInclusiveTime / (double)Entry.m_NumCalls ) * ClockResolution );

		// Max exclusive time in seconds
		Stream.PrintF( "\t%f",  (double)Entry.m_MaxExclusiveTime * ClockResolution );

		// Max inclusive time in seconds
		Stream.PrintF( "\t%f", (double)Entry.m_MaxInclusiveTime * ClockResolution );

		// Number of calls
		Stream.PrintF( "\t%d", Entry.m_NumCalls );

		// Total inclusive time in cycles
		Stream.PrintF( "\t%I64d", Entry.m_TotalInclusiveTime );

		// Total inclusive time in seconds
		Stream.PrintF( "\t%f", (double)Entry.m_TotalInclusiveTime * ClockResolution );

		// Average inclusive time in cycles
		Stream.PrintF( "\t%f", (double)Entry.m_TotalInclusiveTime / (double)Entry.m_NumCalls );

		// Max inclusive time in cycles
		Stream.PrintF( "\t%f", (double)Entry.m_MaxInclusiveTime );

		// Total exclusive time in cycles
		Stream.PrintF( "\t%I64d", Entry.m_TotalExclusiveTime );

		// Total exclusive time in seconds
		Stream.PrintF( "\t%f", (double)Entry.m_TotalExclusiveTime * ClockResolution );

		// Average exclusive time in cycles
		Stream.PrintF( "\t%f", (double)Entry.m_TotalExclusiveTime / (double)Entry.m_NumCalls );

		// Max exclusive time in cycles
		Stream.PrintF( "\t%f", (double)Entry.m_MaxExclusiveTime );

		Stream.WriteInt8( '\n' );
	}
}

void ProfilerTable::Flush()
{
	m_Entries.Clear();
}

const ProfilerMap& ProfilerTable::GetEntries()
{
	return m_Entries;
}

#endif // DO_PROFILING