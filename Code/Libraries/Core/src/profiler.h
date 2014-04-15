#ifndef PROFILER_H
#define PROFILER_H

#define DO_PROFILING ( BUILD_WINDOWS && BUILD_DEV )

#if DO_PROFILING

#include "profilertable.h"
#include "profilerhook.h"

#include <stdlib.h>

#define PROFILE_INTERNAL( entry, hash, hook, permanent ) static const HashedString hash( entry ); ProfilerHook hook( entry, hash, permanent )

// This macro calls the profiler hook's constructor, which starts the clock.
// When the function exits, the destructor is called and finishes the job.
#define PROFILE_FUNCTION PROFILE_INTERNAL( __FUNCSIG__, sFunctionProfilerHash, FunctionProfilerHook, true )
#define FRAME_PROFILE_FUNCTION PROFILE_INTERNAL( __FUNCSIG__, sFunctionFrameProfilerHash, FunctionFrameProfilerHook, false )

// These allow profiling blocks within a function instead of
// the entire function. These can be nested but not overlapped.
#define PROFILE_SCOPE(n) PROFILE_INTERNAL( #n, sScopeProfilerHash, ScopeProfilerHook, true )
#define PROFILE_BEGIN(n) { PROFILE_INTERNAL( #n, s##n##ProfilerHash, n##ProfilerHook, true )
#define PROFILE_END }

#define FRAME_PROFILE_SCOPE(n) PROFILE_INTERNAL( #n, sScopeFrameProfilerHash, ScopeFrameProfilerHook, false )
#define FRAME_PROFILE_BEGIN(n) { PROFILE_INTERNAL( #n, s##n##FrameProfilerHash, n##FrameProfilerHook, false )
#define FRAME_PROFILE_END }

#if BUILD_DEBUG
#define DEBUG_PROFILE_FUNCTION PROFILE_FUNCTION
#define DEBUG_PROFILE_SCOPE PROFILE_SCOPE
#define DEBUG_PROFILE_BEGIN PROFILE_BEGIN
#define DEBUG_PROFILE_END PROFILE_END
#define DEBUG_FRAME_PROFILE_FUNCTION FRAME_PROFILE_FUNCTION
#define DEBUG_FRAME_PROFILE_SCOPE FRAME_PROFILE_SCOPE
#define DEBUG_FRAME_PROFILE_BEGIN FRAME_PROFILE_BEGIN
#define DEBUG_FRAME_PROFILE_END FRAME_PROFILE_END
#else
#define DEBUG_PROFILE_FUNCTION 0
#define DEBUG_PROFILE_SCOPE 0
#define DEBUG_PROFILE_BEGIN(n) 0
#define DEBUG_PROFILE_END 0
#define DEBUG_FRAME_PROFILE_FUNCTION 0
#define DEBUG_FRAME_PROFILE_SCOPE 0
#define DEBUG_FRAME_PROFILE_BEGIN(n) 0
#define DEBUG_FRAME_PROFILE_END 0
#endif

class Clock;
class IDataStream;

class Profiler
{
public:
	static Profiler* GetInstance()
	{
		if( !m_Instance )
		{
			m_Instance = new Profiler;
		}
		return m_Instance;
	}
	static void DeleteInstance()
	{
		if( m_Instance )
		{
			delete m_Instance;
		}
		m_Instance = NULL;
	}

	void Dump( const IDataStream& Stream );
	void Tick();
	void GetFrameStats( ProfilerMap& FrameStats );

	// These are public just for ease and speed (accessing without function overhead), else they'd be private
	Clock*				m_Clock;
	ProfilerTable		m_Table;
	ProfilerTable		m_FrameTable;	// Flushed every frame

private:
	Profiler();
	~Profiler();
	static Profiler*		m_Instance;
};

#else // BUILD_DEV

#define PROFILE_FUNCTION 0
#define PROFILE_SCOPE(n) 0
#define PROFILE_BEGIN(n) 0
#define PROFILE_END 0

#define FRAME_PROFILE_FUNCTION 0
#define FRAME_PROFILE_SCOPE(n) 0
#define FRAME_PROFILE_BEGIN(n) 0
#define FRAME_PROFILE_END 0

#define DEBUG_PROFILE_FUNCTION 0
#define DEBUG_PROFILE_SCOPE(n) 0
#define DEBUG_PROFILE_BEGIN(n) 0
#define DEBUG_PROFILE_END 0

#define DEBUG_FRAME_PROFILE_FUNCTION 0
#define DEBUG_FRAME_PROFILE_SCOPE(n) 0
#define DEBUG_FRAME_PROFILE_BEGIN(n) 0
#define DEBUG_FRAME_PROFILE_END 0

#endif // DO_PROFILING

#endif // PROFILER_H