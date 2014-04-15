#include "core.h"
#include "wbcompeldsensorpoll.h"
#include "configmanager.h"
#include "mathfunc.h"
#include "eldritchframework.h"
#include "clock.h"

WBCompEldSensorPoll::WBCompEldSensorPoll()
:	m_DoPoll( false )
,	m_TickDeltaMin( 0.0f )
,	m_TickDeltaMax( 0.0f )
,	m_NextTickTime( 0.0f )
{
}

WBCompEldSensorPoll::~WBCompEldSensorPoll()
{
}

/*virtual*/ void WBCompEldSensorPoll::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( DoPoll );
	m_DoPoll = ConfigManager::GetInheritedBool( sDoPoll, true, sDefinitionName );

	STATICHASH( TickDeltaMin );
	m_TickDeltaMin = ConfigManager::GetInheritedFloat( sTickDeltaMin, 0.0f, sDefinitionName );

	STATICHASH( TickDeltaMax );
	m_TickDeltaMax = ConfigManager::GetInheritedFloat( sTickDeltaMax, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldSensorPoll::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	Unused( DeltaTime );

	if( m_Paused )
	{
		return;
	}

	if( m_DoPoll )
	{
		const float CurrentTime = GetFramework()->GetClock()->GetGameCurrentTime();
		if( CurrentTime >= m_NextTickTime )
		{
			const float PollDeltaTime = Math::Random( m_TickDeltaMin, m_TickDeltaMax );
			m_NextTickTime = CurrentTime + PollDeltaTime;
			PollTick( PollDeltaTime );
		}
	}
}

/*virtual*/ void WBCompEldSensorPoll::PollTick( const float DeltaTime ) const
{
	Unused( DeltaTime );
}