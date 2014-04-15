#include "core.h"
#include "wbcompeldsleeper.h"
#include "wbcompeldsensorhearing.h"
#include "configmanager.h"
#include "idatastream.h"
#include "wbeventmanager.h"

WBCompEldSleeper::WBCompEldSleeper()
:	m_IsAwake( false )
,	m_NoiseThreshold( 0.0f )
{
	STATIC_HASHED_STRING( OnAINoise );
	GetEventManager()->AddObserver( sOnAINoise, this );
}

WBCompEldSleeper::~WBCompEldSleeper()
{
	WBEventManager* const pEventManager = GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( OnAINoise );
		pEventManager->RemoveObserver( sOnAINoise, this );
	}
}

/*virtual*/ void WBCompEldSleeper::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Awake );
	m_IsAwake	= ConfigManager::GetInheritedBool( sAwake, false, sDefinitionName );

	STATICHASH( NoiseThreshold );
	m_NoiseThreshold = ConfigManager::GetInheritedFloat( sNoiseThreshold, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldSleeper::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnDamaged );
	STATIC_HASHED_STRING( OnAINoise );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnDamaged )
	{
		Wake();
	}
	else if( EventName == sOnAINoise )
	{
		STATIC_HASHED_STRING( EventOwner );
		WBEntity* const pEventOwner = Event.GetEntity( sEventOwner );
		ASSERT( pEventOwner );

		if( pEventOwner == GetEntity() )
		{
			// Ignore AI noises from self (see SensorHearing for more details).
		}
		else
		{
			STATIC_HASHED_STRING( NoiseLocation );
			const Vector NoiseLocation = Event.GetVector( sNoiseLocation );

			STATIC_HASHED_STRING( NoiseRadius );
			const float NoiseRadius = Event.GetFloat( sNoiseRadius );

			HandleNoise( NoiseLocation, NoiseRadius );
		}
	}
}

void WBCompEldSleeper::Wake()
{
	if( m_IsAwake )
	{
		return;
	}

	m_IsAwake = true;

	WB_MAKE_EVENT( OnWoken, GetEntity() );
	WB_DISPATCH_EVENT( GetEventManager(), OnWoken, GetEntity() );
}

void WBCompEldSleeper::HandleNoise( const Vector& NoiseLocation, const float NoiseRadius )
{
	if( m_IsAwake )
	{
		return;
	}

	WBCompEldSensorHearing* const pHearingSensor = GET_WBCOMP( GetEntity(), EldSensorHearing );
	ASSERT( pHearingSensor );

	float Certainty = 0.0f;
	if( !pHearingSensor->GetNoiseCertainty( NoiseLocation, NoiseRadius, Certainty ) )
	{
		return;
	}

	if( Certainty >= m_NoiseThreshold )
	{
		Wake();
	}
}

#define VERSION_EMPTY	0
#define VERSION_AWAKE	1
#define VERSION_CURRENT	1

uint WBCompEldSleeper::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += 1;					// m_IsAwake

	return Size;
}

void WBCompEldSleeper::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteBool( m_IsAwake );
}

void WBCompEldSleeper::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_AWAKE )
	{
		m_IsAwake = Stream.ReadBool();
	}
}