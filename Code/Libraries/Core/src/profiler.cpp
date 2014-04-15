#include "core.h"

#if DO_PROFILING

#include "clock.h"
#include "filestream.h"

#include <time.h>

Profiler* Profiler::m_Instance = NULL;

Profiler::Profiler()
:	m_Clock( NULL )
,	m_Table()
,	m_FrameTable()
{
	m_Clock = new Clock;
}

Profiler::~Profiler()
{
	SafeDelete( m_Clock );
}

void Profiler::Dump( const IDataStream& Stream )
{
	m_Table.Dump( Stream );
}

void Profiler::Tick()
{
	m_FrameTable.Flush();
}

void Profiler::GetFrameStats( ProfilerMap& FrameStats )
{
	FrameStats = m_FrameTable.GetEntries();
}

#endif	// DO_PROFILING