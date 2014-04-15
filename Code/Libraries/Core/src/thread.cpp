#include "core.h"
#include "thread.h"

Thread::Thread( ThreadFunc Function, void* Parameter )
:	m_Thread( NULL )
{
	m_Thread = CreateThread(
		NULL,		// Security attributes--left NULL for default
		0,			// Stack size--left 0 for default
		Function,	// Function
		Parameter,	// Parameter to function
		0,			// Flags--could use CREATE_SUSPENDED and run with ResumeThread
		NULL		// ThreadID (out DWORD, not sure where it's used)
		);
}

Thread::~Thread()
{
	// No need to ExitThread, it happens automatically when the thread function returns
}

void Thread::Wait()
{
	WaitForSingleObject( m_Thread, INFINITE );
}

bool Thread::IsDone()
{
	DWORD Status;
	if( GetExitCodeThread( m_Thread, &Status ) )
	{
		return Status != STILL_ACTIVE;
	}

	return true;
}