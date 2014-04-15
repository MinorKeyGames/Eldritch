#include "core.h"
#include "timer.h"

Timer::Timer()
:	m_TimeRemaining( 0.0f )
{
}

Timer::~Timer()
{
}

void Timer::Start( float Time )
{
	m_TimeRemaining = Time;
}

void Timer::Stop()
{
	m_TimeRemaining = 0.0f;
}

void Timer::AddTime( float Time )
{
	m_TimeRemaining += Time;
}

bool Timer::Tick( float DeltaTime )
{
	if( m_TimeRemaining > 0.0f )
	{
		m_TimeRemaining -= DeltaTime;
		if( m_TimeRemaining <= 0.0f )
		{
			m_TimeRemaining = 0.0f;
			return true;
		}
	}
	return false;
}

bool Timer::HasExpired()
{
	return ( m_TimeRemaining == 0.0f );
}

float Timer::GetTimeRemaining()
{
	return m_TimeRemaining;
}