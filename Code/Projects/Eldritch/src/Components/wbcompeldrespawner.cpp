#include "core.h"
#include "wbcompeldrespawner.h"
#include "wbeventmanager.h"
#include "hashedstring.h"
#include "wbcompeldtransform.h"
#include "configmanager.h"
#include "eldritchworld.h"
#include "eldritchgame.h"
#include "collisioninfo.h"
#include "mathcore.h"
#include "idatastream.h"

WBCompEldRespawner::WBCompEldRespawner()
:	m_OriginSet( false )
,	m_OriginLocation()
,	m_OriginOrientation()
,	m_RetryRespawnTime( 0.0f )
,	m_RespawnMinPlayerDistanceSq( 0.0f )
{
}

WBCompEldRespawner::~WBCompEldRespawner()
{
}

/*virtual*/ void WBCompEldRespawner::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( RetryRespawnTime );
	m_RetryRespawnTime = ConfigManager::GetInheritedFloat( sRetryRespawnTime, 0.0f, sDefinitionName );

	STATICHASH( RespawnMinPlayerDistance );
	m_RespawnMinPlayerDistanceSq = Square( ConfigManager::GetInheritedFloat( sRespawnMinPlayerDistance, 0.0f, sDefinitionName ) );
}


/*virtual*/ void WBCompEldRespawner::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnSpawnedQueued );
	STATIC_HASHED_STRING( Respawn );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnSpawnedQueued )
	{
		ASSERT( !m_OriginSet );	// This isn't really meaningful anymore since I use OnSpawnedQueued instead of OnMoved
		m_OriginSet = true;

		WBCompEldTransform* const	pTransform	= GetEntity()->GetTransformComponent<WBCompEldTransform>();
		DEVASSERT( pTransform );

		m_OriginLocation	= pTransform->GetLocation();
		m_OriginOrientation	= pTransform->GetOrientation();
	}
	else if( EventName == sRespawn )
	{
		TryRespawn();
	}
}

void WBCompEldRespawner::TryRespawn()
{
	if( CanRespawn() )
	{
		Respawn();
	}
	else
	{
		// Requeue event
		WB_MAKE_EVENT( Respawn, GetEntity() );
		WB_QUEUE_EVENT_DELAY( GetEventManager(), Respawn, GetEntity(), m_RetryRespawnTime );
	}
}

bool WBCompEldRespawner::CanRespawn()
{
	// NOTE: I don't care about player facing;
	// player can turn so fast that it's basically irrelevant.
	// We want to spawn when player is distant and occluded.

	if( IsOriginNearPlayer() )
	{
		return false;
	}

	if( CanOriginBeSeenByPlayer() )
	{
		return false;
	}

	return true;
}

bool WBCompEldRespawner::IsOriginNearPlayer()
{
	const Vector PlayerLocation = EldritchGame::GetPlayerLocation();
	const float DistanceSq = ( PlayerLocation - m_OriginLocation ).LengthSquared();
	const bool IsNear = DistanceSq < m_RespawnMinPlayerDistanceSq;

	return IsNear;
}

bool WBCompEldRespawner::CanOriginBeSeenByPlayer()
{
	// Check player occlusion
	CollisionInfo Info;
	Info.m_CollideWorld			= true;
	Info.m_CollideEntities		= true;
	Info.m_UserFlags			= EECF_Occlusion;
	Info.m_StopAtAnyCollision	= true;
	const bool Occluded			= GetWorld()->LineCheck( EldritchGame::GetPlayerViewLocation(), m_OriginLocation, Info );

	return !Occluded;
}

void WBCompEldRespawner::Respawn()
{
	ASSERT( m_OriginSet );

	// Destroy self
	GetEntity()->Destroy();

	WBEntity* const				pSpawnedEntity		= WBWorld::GetInstance()->CreateEntity( GetEntity()->GetName() );

	WBCompEldTransform* const	pSpawnedTransform	= pSpawnedEntity->GetTransformComponent<WBCompEldTransform>();
	ASSERT( pSpawnedTransform );

	pSpawnedTransform->SetLocation(		m_OriginLocation );
	pSpawnedTransform->SetOrientation(	m_OriginOrientation );

	WB_MAKE_EVENT( OnInitialOrientationSet, pSpawnedEntity );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), OnInitialOrientationSet, pSpawnedEntity );
}

#define VERSION_EMPTY	0
#define VERSION_ORIGIN	1
#define VERSION_CURRENT	1

uint WBCompEldRespawner::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += 1;					// m_OriginSet
	Size += sizeof( Vector );	// m_OriginLocation
	Size += sizeof( Angles );	// m_OriginOrientation

	return Size;
}

void WBCompEldRespawner::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteBool( m_OriginSet );
	Stream.Write( sizeof( Vector ), &m_OriginLocation );
	Stream.Write( sizeof( Angles ), &m_OriginOrientation );
}

void WBCompEldRespawner::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_ORIGIN )
	{
		m_OriginSet = Stream.ReadBool();
		Stream.Read( sizeof( Vector ), &m_OriginLocation );
		Stream.Read( sizeof( Angles ), &m_OriginOrientation );
	}
}