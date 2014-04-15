#ifndef PROFILERHOOK_H
#define PROFILERHOOK_H

#include "hashedstring.h"

class ProfilerHook
{
public:
	ProfilerHook( const char* Name, const HashedString& HashedName, bool Permanent );
	~ProfilerHook();

private:
	const char*		m_Name;
	HashedString	m_HashedName;
	__int64			m_SetupTime;
	__int64			m_RunningStartTime;
	__int64			m_RunningTime;
	bool			m_Permanent;
};

#endif // PROFILER_H