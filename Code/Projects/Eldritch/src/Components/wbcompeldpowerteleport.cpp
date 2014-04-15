#include "core.h"
#include "wbcompeldpowerteleport.h"
#include "wbeventmanager.h"
#include "idatastream.h"
#include "Components/wbcompeldtransform.h"
#include "Components/wbcompeldcollision.h"
#include "Components/wbcompowner.h"
#include "eldritchworld.h"
#include "collisioninfo.h"

WBCompEldPowerTeleport::WBCompEldPowerTeleport()
:	m_Beacon()
{
}

WBCompEldPowerTeleport::~WBCompEldPowerTeleport()
{
}

/*virtual*/ void WBCompEldPowerTeleport::HandleEvent( const WBEvent& Event )
{
	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnSpawnedEntityAction );
	STATIC_HASHED_STRING( TryTeleport );
	STATIC_HASHED_STRING( DestroyBeacon );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnSpawnedEntityAction )
	{
		STATIC_HASHED_STRING( SpawnedEntity );
		WBEntity* const pSpawnedEntity = Event.GetEntity( sSpawnedEntity );

		ASSERT( m_Beacon.Get() == NULL );
		m_Beacon = pSpawnedEntity;
	}
	else if( EventName == sTryTeleport )
	{
		TryTeleport();
	}
	else if( EventName == sDestroyBeacon )
	{
		WBEntity* const pBeacon = m_Beacon.Get();
		if( pBeacon )
		{
			pBeacon->Destroy();
		}
	}
}

void WBCompEldPowerTeleport::TryTeleport() const
{
	WBEntity* const				pBeacon				= m_Beacon.Get();
	if( !pBeacon )
	{
		WARN;
		return;
	}

	WBEntity* const				pEntity				= GetEntity();

	WBCompEldTransform* const	pBeaconTransform	= pBeacon->GetTransformComponent<WBCompEldTransform>();
	ASSERT( pBeaconTransform );

	WBEntity* const				pOwner				= WBCompOwner::GetTopmostOwner( pEntity );
	ASSERT( pOwner );

	WBCompEldTransform* const	pOwnerTransform		= pOwner->GetTransformComponent<WBCompEldTransform>();
	ASSERT( pOwnerTransform );

	WBCompEldCollision* const	pOwnerCollision		= GET_WBCOMP( pOwner, EldCollision );
	ASSERT( pOwnerCollision );

	Vector						TeleportLocation	= pBeaconTransform->GetLocation();
	const Vector				OwnerExtents		= pOwnerCollision->GetExtents();

	EldritchWorld* const		pWorld				= GetWorld();
	ASSERT( pWorld );

	CollisionInfo Info;
	Info.m_CollideWorld		= true;
	Info.m_CollideEntities	= true;
	Info.m_CollidingEntity	= pOwner;
	Info.m_UserFlags		= EECF_EntityCollision;

	if( !pWorld->FindSpot( TeleportLocation, OwnerExtents, Info ) )
	{
		return;
	}

	pOwnerTransform->SetLocation( TeleportLocation );

	// Notify that teleport was successful
	WB_MAKE_EVENT( OnTeleported, pEntity );
	WB_DISPATCH_EVENT( GetEventManager(), OnTeleported, pEntity );
}

#define VERSION_EMPTY	0
#define VERSION_BEACON	1
#define VERSION_CURRENT	1

/*virtual*/ uint WBCompEldPowerTeleport::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;						// Version
	Size += sizeof( WBEntityRef );	// m_Beacon

	return Size;
}

/*virtual*/ void WBCompEldPowerTeleport::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.Write( sizeof( WBEntityRef ), &m_Beacon );
}

/*virtual*/ void WBCompEldPowerTeleport::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_BEACON )
	{
		Stream.Read( sizeof( WBEntityRef ), &m_Beacon );
	}
}