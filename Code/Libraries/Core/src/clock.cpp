#include "core.h"
#include "clock.h"
#include "simplestring.h"
#include "configmanager.h"

#if BUILD_WINDOWS_NO_SDL
#include <Windows.h>
#endif

Clock::Clock()
:	m_PhysicalBaseTime( 0 )
,	m_PhysicalCurrentTime( 0 )
,	m_PhysicalDeltaTime( 0 )
,	m_MachineCurrentTime( 0.0f )
,	m_MachineDeltaTime( 0.0f )
,	m_GameCurrentTime( 0.0f )
,	m_GameDeltaTime( 0.0f )
,	m_TickCount( 0 )
,	m_Resolution( 0.0 )
,	m_MultiplierRequests()
,	m_GameDeltaTimeMax( 0.0f )
,	m_UseFixedMachineDeltaTime( false )
,	m_FixedMachineDeltaTime( 0.0f )
{
	Initialize();
}

Clock::~Clock()
{
	ShutDown();
}

void Clock::Initialize()
{
	DEBUGPRINTF( "Initializing Clock\n" );

	ShutDown();

#if BUILD_WINDOWS_NO_SDL
	LARGE_INTEGER Frequency;
	LARGE_INTEGER Counter;

	QueryPerformanceFrequency( &Frequency );
	QueryPerformanceCounter( &Counter );

	m_Resolution			= 1.0 / (double)Frequency.QuadPart;
	m_PhysicalBaseTime		= Counter.QuadPart;
#endif

#if BUILD_SDL
	Uint64 Frequency	= SDL_GetPerformanceFrequency();
	Uint64 Counter		= SDL_GetPerformanceCounter();

	m_Resolution			= 1.0 / static_cast<double>( Frequency );
	m_PhysicalBaseTime		= Counter;
#endif

	m_PhysicalDeltaTime		= 0;
	m_MachineDeltaTime		= 0.0f;
	m_GameDeltaTime			= 0.0f;

	m_PhysicalCurrentTime	= 0;
	m_MachineCurrentTime	= 0.0f;
	m_GameCurrentTime		= 0.0f;

	m_TickCount				= 0;
}

void Clock::ShutDown()
{
	ClearMultiplierRequests();
}

void Clock::Tick( bool GamePaused /*= false*/, const bool DoGameTick /*= true*/ )
{
	XTRACE_FUNCTION;

#if BUILD_WINDOWS_NO_SDL
	LARGE_INTEGER Counter;
	QueryPerformanceCounter( &Counter );

	CLOCK_T PhysicalTime	= ( Counter.QuadPart - m_PhysicalBaseTime );
#endif

#if BUILD_SDL
	Uint64 Counter = SDL_GetPerformanceCounter();

	CLOCK_T PhysicalTime	= ( Counter - m_PhysicalBaseTime );
#endif

	m_PhysicalDeltaTime		= PhysicalTime - m_PhysicalCurrentTime;
	m_MachineDeltaTime		= m_UseFixedMachineDeltaTime ? m_FixedMachineDeltaTime : CounterToSeconds( m_PhysicalDeltaTime );

	m_PhysicalCurrentTime	= PhysicalTime;
	m_MachineCurrentTime	= m_UseFixedMachineDeltaTime ? ( m_MachineCurrentTime + m_FixedMachineDeltaTime ) : CounterToSeconds( m_PhysicalCurrentTime );

	if( DoGameTick )
	{
		if( GamePaused )
		{
			m_GameDeltaTime = 0.0f;
		}
		else if( m_MultiplierRequests.Size() )
		{
			TickMultiplierRequests( m_MachineDeltaTime );
			m_GameDeltaTime		= m_MachineDeltaTime * GetMultiplier();
		}
		else
		{
			m_GameDeltaTime = m_MachineDeltaTime;
		}

		// Clamp the upper range of game delta time to prevent hitches from
		// causing instability in the sim. This is done here instead of when
		// returning the value so that it actually does affect the game
		// current time as well.
		if( m_GameDeltaTimeMax > 0.0f && m_GameDeltaTime > m_GameDeltaTimeMax )
		{
			m_GameDeltaTime = m_GameDeltaTimeMax;
		}

		m_GameCurrentTime		= m_GameCurrentTime + m_GameDeltaTime;	// NOTE: Accumulates drift
	}

	++m_TickCount;
}

void Clock::GameTick( const bool GamePaused, const float DeltaTime )
{
	XTRACE_FUNCTION;

	if( GamePaused )
	{
		m_GameDeltaTime = 0.0f;
	}
	else
	{
		m_GameDeltaTime		= DeltaTime;
		m_GameCurrentTime	= m_GameCurrentTime + m_GameDeltaTime;	// NOTE: Accumulates drift
	}
}

float Clock::GetGameCurrentTime() const
{
	return m_GameCurrentTime;
}

float Clock::GetGameDeltaTime() const
{
	return m_GameDeltaTime;
}

float Clock::GetMachineCurrentTime() const
{
	return m_MachineCurrentTime;
}

float Clock::GetMachineDeltaTime() const
{
	return m_MachineDeltaTime;
}

CLOCK_T Clock::GetPhysicalCurrentTime() const
{
	return m_PhysicalCurrentTime;
}

CLOCK_T Clock::GetPhysicalDeltaTime() const
{
	return m_PhysicalDeltaTime;
}

// Used by profiler, could probably be compiled out for Release
double Clock::GetResolution() const
{
	return m_Resolution;
}

uint Clock::GetTickCount() const
{
	return m_TickCount;
}

inline float Clock::CounterToSeconds( const CLOCK_T Counter ) const
{
	return (float)( (double)Counter * m_Resolution );
}

void Clock::Report() const
{
	DEBUGPRINTF( "=== Clock report ===\n" );
	DEBUGPRINTF( "Machine time: %.2f\n", m_MachineCurrentTime );
	DEBUGPRINTF( "Game time: %.2f\n", m_GameCurrentTime );
	DEBUGPRINTF( "Machine delta time: %.4f\n", m_MachineDeltaTime );
	DEBUGPRINTF( "Game delta time: %.4f\n", m_GameDeltaTime );
	DEBUGPRINTF( "Game time drift: %.4f\n", m_GameCurrentTime - m_MachineCurrentTime );	// Only meaningful before time effects are applied
}

Clock::MultiplierRequest* Clock::AddMultiplierRequest( const SimpleString& DefinitionName )
{
	STATICHASH( Duration );
	STATICHASH( Multiplier );
	MAKEHASH( DefinitionName );
	return AddMultiplierRequest(
		ConfigManager::GetFloat( sDuration, 0.0f, sDefinitionName ),
		ConfigManager::GetFloat( sMultiplier, 0.0f, sDefinitionName ) );
}

Clock::MultiplierRequest* Clock::AddMultiplierRequest( float Duration, float Multiplier )
{
	if( Multiplier < 0.0f )
	{
		Multiplier = 0.0f;
		WARNDESC( "Clock multiplier should not be less than zero!" );
	}

	MultiplierRequest* pMultiplierRequest = new MultiplierRequest;
	pMultiplierRequest->m_Duration = Duration;
	pMultiplierRequest->m_Multiplier = Multiplier;
	m_MultiplierRequests.PushBack( pMultiplierRequest );

	return pMultiplierRequest;
}

void Clock::RemoveMultiplierRequest( Clock::MultiplierRequest** pMultiplierRequest )
{
	// For safety, check that the request is actually still in the list before deleting.
	// (Else, it might have already been removed via ClearMultiplierRequests, and the
	// pointer could be dangling.)
	for( List< MultiplierRequest* >::Iterator MultiplierIterator = m_MultiplierRequests.Begin(); MultiplierIterator != m_MultiplierRequests.End(); )
	{
		if( *pMultiplierRequest == *MultiplierIterator )
		{
			m_MultiplierRequests.Pop( MultiplierIterator );
			SafeDelete( *pMultiplierRequest );
			break;
		}
		else
		{
			++MultiplierIterator;
		}
	}

	// Null the request either way
	*pMultiplierRequest = NULL;
}

void Clock::ClearMultiplierRequests()
{
	FOR_EACH_LIST( MultiplierIterator, m_MultiplierRequests, MultiplierRequest* )
	{
		SafeDelete( *MultiplierIterator );
	}
	m_MultiplierRequests.Clear();
}

float Clock::GetMultiplier() const
{
	if( m_MultiplierRequests.Size() == 0 )
	{
		return 1.0f;
	}

	float Min = -1.0f;
	FOR_EACH_LIST( MultiplierIterator, m_MultiplierRequests, MultiplierRequest* )
	{
		MultiplierRequest* pMultiplierRequest = *MultiplierIterator;
		if( Min < 0.0f || pMultiplierRequest->m_Multiplier < Min )
		{
			Min = pMultiplierRequest->m_Multiplier;
		}
	}
	return Min;
}

void Clock::TickMultiplierRequests( float DeltaTime )
{
	for( List< MultiplierRequest* >::Iterator MultiplierIterator = m_MultiplierRequests.Begin(); MultiplierIterator != m_MultiplierRequests.End(); )
	{
		MultiplierRequest* pMultiplierRequest = *MultiplierIterator;
		// If the request has a duration equal to or less than zero, it is infinite
		if( pMultiplierRequest->m_Duration > 0.0f )
		{
			pMultiplierRequest->m_Duration -= DeltaTime;
			if( pMultiplierRequest->m_Duration <= 0.0f )
			{
				m_MultiplierRequests.Pop( MultiplierIterator );
				SafeDelete( pMultiplierRequest );
				continue;
			}
		}
		++MultiplierIterator;
	}
}

void Clock::SetGameDeltaTimeMax( float Limit )
{
	m_GameDeltaTimeMax = Limit;
}

void Clock::UseFixedMachineDeltaTime( float FixedDeltaTime )
{
	m_UseFixedMachineDeltaTime = true;
	m_FixedMachineDeltaTime = FixedDeltaTime;
}

void Clock::UseActualMachineDeltaTime()
{
	m_UseFixedMachineDeltaTime = false;
}