#include "core.h"
#include "mutex.h"

Mutex::Mutex()
:	m_hMutex( 0 )
{
}

Mutex::~Mutex()
{
	if( m_hMutex )
	{
		Destroy();
	}
}

void Mutex::Create()
{
	m_hMutex = CreateMutex( NULL, FALSE, NULL );
	ASSERT( m_hMutex );
}

void Mutex::Wait()
{
	if( m_hMutex )
	{
		WaitForSingleObject( m_hMutex, INFINITE );
	}
}

void Mutex::Release()
{
	if( m_hMutex )
	{
		ReleaseMutex( m_hMutex );
	}
}

void Mutex::Destroy()
{
	if( m_hMutex )
	{
		CloseHandle( m_hMutex );
		m_hMutex = 0;
	}
}