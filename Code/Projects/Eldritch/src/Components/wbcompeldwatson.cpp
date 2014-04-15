#include "core.h"
#include "wbcompeldwatson.h"
#include "configmanager.h"
#include "wbeventmanager.h"
#include "idatastream.h"
#include "eldritchgame.h"
#include "wbcompeldsensorvision.h"
#include "wbcompeldtransform.h"
#include "wbcompeldcollision.h"
#include "mathcore.h"
#include "eldritchworld.h"
#include "collisioninfo.h"

WBCompEldWatson::WBCompEldWatson()
:	m_Activated( false )
,	m_Primed( false )
,	m_MinAttackDistSq( 0.0f )
,	m_MaxAttackDistSq( 0.0f )
,	m_TeleportDist( 0.0f )
,	m_EyeOffsetZ( 0.0f )
{
}

WBCompEldWatson::~WBCompEldWatson()
{
}

/*virtual*/ void WBCompEldWatson::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( MinAttackDist );
	m_MinAttackDistSq = Square( ConfigManager::GetInheritedFloat( sMinAttackDist, 0.0f, sDefinitionName ) );

	STATICHASH( MaxAttackDist );
	m_MaxAttackDistSq = Square( ConfigManager::GetInheritedFloat( sMaxAttackDist, 0.0f, sDefinitionName ) );

	STATICHASH( TeleportDist );
	m_TeleportDist = ConfigManager::GetInheritedFloat( sTeleportDist, 0.0f, sDefinitionName );

	STATICHASH( EyeOffsetZ );
	m_EyeOffsetZ = ConfigManager::GetInheritedFloat( sEyeOffsetZ, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldWatson::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	if( m_Primed )
	{
		TickPrimed();
	}
	else if( m_Activated )
	{
		TickActivated();
	}
	else
	{
		TickUnactivated();
	}
}

void WBCompEldWatson::TickPrimed()
{
	if( !EldritchGame::IsPlayerAlive() )
	{
		// Player can't cause Watson to teleport and attack when dead
		return;
	}

	if( !EldritchGame::IsPlayerVisible() )
	{
		// Player can't cause Watson to teleport and attack when invisible
		return;
	}

	if( !IsPlayerVulnerable( true ) )
	{
		// Do nothing, can't attack until player is vulnerable
		return;
	}

	WBEntity* const pEntity = GetEntity();
	DEVASSERT( pEntity );

	WBEntity* const pPlayer = EldritchGame::GetPlayer();
	ASSERT( pPlayer );

	WBCompEldTransform* const pTransform = pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldTransform* const pPlayerTransform = pPlayer->GetTransformComponent<WBCompEldTransform>();
	ASSERT( pPlayerTransform );

	const Vector			PlayerLocation		= pPlayerTransform->GetLocation();
	const Angles			PlayerOrientation	= pPlayerTransform->GetOrientation();
	EldritchWorld* const	pWorld				= GetWorld();
	ASSERT( pWorld );

	const Angles				PlayerOrientation2D	= Angles( 0.0f, 0.0f, PlayerOrientation.Yaw );
	const Vector				PlayerFacing2D		= PlayerOrientation2D.ToVector();
	WBCompEldCollision* const	pCollision			= GET_WBCOMP( pEntity, EldCollision );
	const Vector				Extents				= pCollision->GetExtents();
	Vector						TeleportLocation	= PlayerLocation - ( PlayerFacing2D * m_TeleportDist );
	CollisionInfo FindSpotInfo;
	FindSpotInfo.m_CollideWorld		= true;
	FindSpotInfo.m_CollideEntities	= true;
	FindSpotInfo.m_UserFlags		= EECF_EntityCollision;
	FindSpotInfo.m_CollidingEntity	= pEntity;

	if( pCollision->MatchesAllCollisionFlags( EECF_BlocksEntities ) )
	{
		// HACK: Test against all entities, since Watson is a blocking entity.
		FindSpotInfo.m_UserFlags |= EECF_CollideAllEntities;
	}

	if( !pWorld->FindSpot( TeleportLocation, Extents, FindSpotInfo ) )
	{
		// Do nothing, we don't fit behind player
		return;
	}

	const Vector				NewToPlayer2D		= ( PlayerLocation - TeleportLocation ).Get2D();
	const Angles				NewOrientation		= NewToPlayer2D.ToAngles();

	pTransform->SetLocation( TeleportLocation );
	pTransform->SetOrientation( NewOrientation );

	WB_MAKE_EVENT( OnWatsonTeleported, GetEntity() );
	WB_DISPATCH_EVENT( GetEventManager(), OnWatsonTeleported, GetEntity() );

	// Need to prime again before we can do another teleport
	m_Primed = false;
}

void WBCompEldWatson::TickActivated()
{
	if( !EldritchGame::IsPlayerAlive() )
	{
		// Player can't cause Watson to prime when dead
		return;
	}

	if( !EldritchGame::IsPlayerVisible() )
	{
		// Player can't cause Watson to prime when invisible
		return;
	}

	// Don't check min distance, because we don't want to re-prime right after an attack.
	if( IsPlayerVulnerable( false ) )
	{
		return;
	}

	// We're activated and the player has (probably) seen us. Primed for attack.
	m_Primed = true;

	WB_MAKE_EVENT( OnWatsonPrimed, GetEntity() );
	WB_DISPATCH_EVENT( GetEventManager(), OnWatsonPrimed, GetEntity() );
}

bool WBCompEldWatson::IsPlayerVulnerable( const bool CheckMinDistance ) const
{
	WBEntity* const pEntity = GetEntity();
	DEVASSERT( pEntity );

	WBEntity* const pPlayer = EldritchGame::GetPlayer();
	ASSERT( pPlayer );

	WBCompEldTransform* const pTransform = pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldTransform* const pPlayerTransform = pPlayer->GetTransformComponent<WBCompEldTransform>();
	ASSERT( pPlayerTransform );

	const Vector	EyeLocation			= pTransform->GetLocation() + Vector( 0.0f, 0.0f, m_EyeOffsetZ );
	const Vector	PlayerLocation		= pPlayerTransform->GetLocation();
	const Vector	PlayerViewLocation	= EldritchGame::GetPlayerViewLocation();
	const Vector	ToPlayer			= PlayerViewLocation - EyeLocation;
	const float		DistanceSq			= ToPlayer.LengthSquared();

	if( CheckMinDistance && DistanceSq < m_MinAttackDistSq )
	{
		// Do nothing, we're already close to player.
		return false;
	}

	if( DistanceSq > m_MaxAttackDistSq )
	{
		// Do nothing, we're too far from the player
		return false;
	}

	// Do both a 3D and 2D check. 3D check is necessary so Watson doesn't teleport
	// when player is looking up or down at it, and 2D is necessary so Watson doesn't
	// teleport when player is looking up or down *away* from it.
	const Angles	PlayerOrientation	= pPlayerTransform->GetOrientation();
	const Angles	PlayerOrientation2D	= Angles( 0.0f, 0.0f, PlayerOrientation.Yaw );
	const Vector	PlayerFacing		= PlayerOrientation.ToVector();
	const Vector	PlayerFacing2D		= PlayerOrientation2D.ToVector();
	const Vector	ToPlayer2D			= ToPlayer.Get2D();
	const float		CosPlayerAngle		= PlayerFacing.Dot( ToPlayer );
	const float		CosPlayerAngle2D	= PlayerFacing2D.Dot( ToPlayer2D );
	if( CosPlayerAngle < 0.0f || CosPlayerAngle2D < 0.0f )
	{
		// Do nothing, player is facing us.
		return false;
	}

	EldritchWorld* const	pWorld	= GetWorld();
	ASSERT( pWorld );

	CollisionInfo OcclusionInfo;
	OcclusionInfo.m_CollideWorld		= true;
	OcclusionInfo.m_CollideEntities		= true;
	OcclusionInfo.m_UserFlags			= EECF_Occlusion;
	OcclusionInfo.m_StopAtAnyCollision	= true;
	if( pWorld->LineCheck( EyeLocation, PlayerViewLocation, OcclusionInfo ) )
	{
		// Do nothing, player is occluded.
		return false;
	}

	return true;
}

void WBCompEldWatson::TickUnactivated()
{
	WBEntity* const pPlayer = EldritchGame::GetPlayer();
	ASSERT( pPlayer );

	WBCompEldSensorVision* const pVision = GET_WBCOMP( GetEntity(), EldSensorVision );
	ASSERT( pVision );

	if( pVision->IsVisible( pPlayer ) )
	{
		m_Activated = true;

		WB_MAKE_EVENT( OnWatsonActivated, GetEntity() );
		WB_DISPATCH_EVENT( GetEventManager(), OnWatsonActivated, GetEntity() );
	}
}

#define VERSION_EMPTY		0
#define VERSION_ACTIVATED	1
#define VERSION_PRIMED		2
#define VERSION_CURRENT		2

uint WBCompEldWatson::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;	// Version
	Size += 1;	// m_Activated
	Size += 1;	// m_Primed

	return Size;
}

void WBCompEldWatson::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteBool( m_Activated );

	Stream.WriteBool( m_Primed );
}

void WBCompEldWatson::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_ACTIVATED )
	{
		m_Activated = Stream.ReadBool();
	}

	if( Version >= VERSION_PRIMED )
	{
		m_Primed = Stream.ReadBool();
	}
}