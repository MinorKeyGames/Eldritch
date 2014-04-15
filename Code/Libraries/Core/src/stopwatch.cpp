#include "core.h"
#include "stopwatch.h"

Stopwatch::Stopwatch()
:	m_Clock()
,	m_PauseRequest( NULL )
{
}

float Stopwatch::GetTime()
{
	m_Clock.Tick();
	return m_Clock.GetGameCurrentTime();
}

void Stopwatch::TogglePause()
{
	m_Clock.Tick();

	if( m_PauseRequest )
	{
		// Currently paused, unpause
		m_Clock.RemoveMultiplierRequest( &m_PauseRequest );
	}
	else
	{
		m_PauseRequest = m_Clock.AddMultiplierRequest( 0.0f, 0.0f );
	}
}

void Stopwatch::Reset()
{
	m_Clock.Initialize();
	m_PauseRequest = NULL;
}