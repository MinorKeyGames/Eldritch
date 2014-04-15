#include "core.h"
#include "wbcompelddoor.h"
#include "wbcompeldmesh.h"
#include "wbcompeldcollision.h"
#include "wbcompeldtransform.h"
#include "wbcompeldfrobbable.h"
#include "wbeventmanager.h"
#include "eldritchworld.h"
#include "collisioninfo.h"
#include "configmanager.h"
#include "idatastream.h"
#include "aabb.h"
#include "mathcore.h"
#include "eldritchnav.h"

WBCompEldDoor::WBCompEldDoor()
:	m_Open( false )
,	m_Locked( false )
,	m_InterpTime( 0.0f )
,	m_ClosedOffset()
,	m_ClosedOrientation()
,	m_ClosedFrobOffset()
,	m_ClosedFrobExtents()
,	m_ClosedIrradianceOffset()
,	m_OpenOffset()
,	m_OpenOrientation()
,	m_OpenFrobOffset()
,	m_OpenFrobExtents()
,	m_OpenIrradianceOffset()
,	m_OffsetInterpolator()
,	m_OrientationInterpolator()
,	m_UnlockedMesh()
,	m_LockedMesh()
,	m_UnlockedTexture()
,	m_LockedTexture()
,	m_UnlockedFriendlyName()
,	m_LockedFriendlyName()
{
}

WBCompEldDoor::~WBCompEldDoor()
{
}

/*virtual*/ void WBCompEldDoor::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( InterpTime );
	m_InterpTime = ConfigManager::GetInheritedFloat( sInterpTime, 0.0f, sDefinitionName );

	STATICHASH( ClosedOffsetX );
	m_ClosedOffset.x = ConfigManager::GetInheritedFloat( sClosedOffsetX, 0.0f, sDefinitionName );

	STATICHASH( ClosedOffsetY );
	m_ClosedOffset.y = ConfigManager::GetInheritedFloat( sClosedOffsetY, 0.0f, sDefinitionName );

	STATICHASH( ClosedOffsetZ );
	m_ClosedOffset.z = ConfigManager::GetInheritedFloat( sClosedOffsetZ, 0.0f, sDefinitionName );

	STATICHASH( ClosedAngle );
	m_ClosedOrientation.Yaw = DEGREES_TO_RADIANS( ConfigManager::GetInheritedFloat( sClosedAngle, 0.0f, sDefinitionName ) );

	STATICHASH( ClosedFrobOffsetX );
	m_ClosedFrobOffset.x = ConfigManager::GetInheritedFloat( sClosedFrobOffsetX, 0.0f, sDefinitionName );

	STATICHASH( ClosedFrobOffsetY );
	m_ClosedFrobOffset.y = ConfigManager::GetInheritedFloat( sClosedFrobOffsetY, 0.0f, sDefinitionName );

	STATICHASH( ClosedFrobOffsetZ );
	m_ClosedFrobOffset.z = ConfigManager::GetInheritedFloat( sClosedFrobOffsetZ, 0.0f, sDefinitionName );

	STATICHASH( ClosedFrobBoxX );
	m_ClosedFrobExtents.x = ConfigManager::GetInheritedFloat( sClosedFrobBoxX, 0.0f, sDefinitionName );

	STATICHASH( ClosedFrobBoxY );
	m_ClosedFrobExtents.y = ConfigManager::GetInheritedFloat( sClosedFrobBoxY, 0.0f, sDefinitionName );

	STATICHASH( ClosedFrobBoxZ );
	m_ClosedFrobExtents.z = ConfigManager::GetInheritedFloat( sClosedFrobBoxZ, 0.0f, sDefinitionName );

	STATICHASH( ClosedIrrX );
	m_ClosedIrradianceOffset.x = ConfigManager::GetInheritedFloat( sClosedIrrX, 0.0f, sDefinitionName );

	STATICHASH( ClosedIrrY );
	m_ClosedIrradianceOffset.y = ConfigManager::GetInheritedFloat( sClosedIrrY, 0.0f, sDefinitionName );

	STATICHASH( ClosedIrrZ );
	m_ClosedIrradianceOffset.z = ConfigManager::GetInheritedFloat( sClosedIrrZ, 0.0f, sDefinitionName );

	STATICHASH( OpenOffsetX );
	m_OpenOffset.x = ConfigManager::GetInheritedFloat( sOpenOffsetX, 0.0f, sDefinitionName );

	STATICHASH( OpenOffsetY );
	m_OpenOffset.y = ConfigManager::GetInheritedFloat( sOpenOffsetY, 0.0f, sDefinitionName );

	STATICHASH( OpenOffsetZ );
	m_OpenOffset.z = ConfigManager::GetInheritedFloat( sOpenOffsetZ, 0.0f, sDefinitionName );

	STATICHASH( OpenAngle );
	m_OpenOrientation.Yaw = DEGREES_TO_RADIANS( ConfigManager::GetInheritedFloat( sOpenAngle, 0.0f, sDefinitionName ) );

	STATICHASH( OpenFrobOffsetX );
	m_OpenFrobOffset.x = ConfigManager::GetInheritedFloat( sOpenFrobOffsetX, 0.0f, sDefinitionName );

	STATICHASH( OpenFrobOffsetY );
	m_OpenFrobOffset.y = ConfigManager::GetInheritedFloat( sOpenFrobOffsetY, 0.0f, sDefinitionName );

	STATICHASH( OpenFrobOffsetZ );
	m_OpenFrobOffset.z = ConfigManager::GetInheritedFloat( sOpenFrobOffsetZ, 0.0f, sDefinitionName );

	STATICHASH( OpenFrobBoxX );
	m_OpenFrobExtents.x = ConfigManager::GetInheritedFloat( sOpenFrobBoxX, 0.0f, sDefinitionName );

	STATICHASH( OpenFrobBoxY );
	m_OpenFrobExtents.y = ConfigManager::GetInheritedFloat( sOpenFrobBoxY, 0.0f, sDefinitionName );

	STATICHASH( OpenFrobBoxZ );
	m_OpenFrobExtents.z = ConfigManager::GetInheritedFloat( sOpenFrobBoxZ, 0.0f, sDefinitionName );

	STATICHASH( OpenIrrX );
	m_OpenIrradianceOffset.x = ConfigManager::GetInheritedFloat( sOpenIrrX, 0.0f, sDefinitionName );

	STATICHASH( OpenIrrY );
	m_OpenIrradianceOffset.y = ConfigManager::GetInheritedFloat( sOpenIrrY, 0.0f, sDefinitionName );

	STATICHASH( OpenIrrZ );
	m_OpenIrradianceOffset.z = ConfigManager::GetInheritedFloat( sOpenIrrZ, 0.0f, sDefinitionName );

	STATICHASH( Locked );
	m_Locked = ConfigManager::GetInheritedBool( sLocked, false, sDefinitionName );

	STATICHASH( UnlockedMesh );
	m_UnlockedMesh = ConfigManager::GetInheritedString( sUnlockedMesh, "", sDefinitionName );

	STATICHASH( LockedMesh );
	m_LockedMesh = ConfigManager::GetInheritedString( sLockedMesh, "", sDefinitionName );

	STATICHASH( UnlockedTexture );
	m_UnlockedTexture = ConfigManager::GetInheritedString( sUnlockedTexture, "", sDefinitionName );

	STATICHASH( LockedTexture );
	m_LockedTexture = ConfigManager::GetInheritedString( sLockedTexture, "", sDefinitionName );

	STATICHASH( UnlockedFriendlyName );
	m_UnlockedFriendlyName = ConfigManager::GetInheritedString( sUnlockedFriendlyName, "", sDefinitionName );

	STATICHASH( LockedFriendlyName );
	m_LockedFriendlyName = ConfigManager::GetInheritedString( sLockedFriendlyName, "", sDefinitionName );
}

WBCompEldDoor::EDoorRotation WBCompEldDoor::GetRotation() const
{
	WBCompEldTransform* const	pTransform	= GetEntity()->GetTransformComponent<WBCompEldTransform>();
	const float					Yaw			= pTransform->GetOrientation().Yaw;

	if( Equal( Yaw, DEGREES_TO_RADIANS( 0.0f ) ) )		{ return EDR_0; }
	if( Equal( Yaw, DEGREES_TO_RADIANS( 90.0f ) ) )		{ return EDR_90; }
	if( Equal( Yaw, DEGREES_TO_RADIANS( 180.0f ) ) )	{ return EDR_180; }
	if( Equal( Yaw, DEGREES_TO_RADIANS( 270.0f ) ) )	{ return EDR_270; }

	WARN;
	return EDR_0;
}

void WBCompEldDoor::AdjustForFacing()
{
	EDoorRotation Rotation = GetRotation();

	if( Rotation == EDR_0 )
	{
		return;
	}

	m_ClosedOffset				= RotateOffset(		m_ClosedOffset,				Rotation );
	m_ClosedOrientation.Yaw		= RotateYaw(		m_ClosedOrientation.Yaw,	Rotation );
	m_ClosedFrobOffset			= RotateOffset(		m_ClosedFrobOffset,			Rotation );
	m_ClosedFrobExtents			= RotateExtents(	m_ClosedFrobExtents,		Rotation );
	m_ClosedIrradianceOffset	= RotateOffset(		m_ClosedIrradianceOffset,	Rotation );
	m_OpenOffset				= RotateOffset(		m_OpenOffset,				Rotation );
	m_OpenOrientation.Yaw		= RotateYaw(		m_OpenOrientation.Yaw,		Rotation );
	m_OpenFrobOffset			= RotateOffset(		m_OpenFrobOffset,			Rotation );
	m_OpenFrobExtents			= RotateExtents(	m_OpenFrobExtents,			Rotation );
	m_OpenIrradianceOffset		= RotateOffset(		m_OpenIrradianceOffset,		Rotation );

	// NOTE: Collision gets updated from facing in EldCollision component
}

Vector WBCompEldDoor::RotateExtents( const Vector& Extents, const EDoorRotation Rotation ) const
{
	ASSERT( Rotation != EDR_0 );

	return ( Rotation == EDR_180 ) ? Extents : Vector( Extents.y, Extents.x, Extents.z );
}

Vector WBCompEldDoor::RotateOffset( const Vector& Offset, const EDoorRotation Rotation ) const
{
	ASSERT( Rotation != EDR_0 );

	if( Rotation == EDR_90 )	{ return Vector(	-Offset.y,	Offset.x,	Offset.z ); }
	if( Rotation == EDR_180 )	{ return Vector(	-Offset.x,	-Offset.y,	Offset.z ); }
	if( Rotation == EDR_270 )	{ return Vector(	Offset.y,	-Offset.x,	Offset.z ); }

	WARN;
	return Offset;
}

float WBCompEldDoor::RotateYaw( const float Yaw, const EDoorRotation Rotation ) const
{
	ASSERT( Rotation != EDR_0 );

	if( Rotation == EDR_90 )	{ return Yaw + PI * 0.5f; }
	if( Rotation == EDR_180 )	{ return Yaw + PI; }
	if( Rotation == EDR_270 )	{ return Yaw + PI * 1.5f; }

	WARN;
	return Yaw;
}

/*virtual*/ void WBCompEldDoor::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( TryToggle );
	STATIC_HASHED_STRING( OnInitialOrientationSet );
	STATIC_HASHED_STRING( OnLoaded );
	STATIC_HASHED_STRING( Lock );
	STATIC_HASHED_STRING( Unlock );

	const HashedString EventName = Event.GetEventName();

	if( EventName == sTryToggle )
	{
		STATIC_HASHED_STRING( Frobber );
		WBEntity* const pFrobber = Event.GetEntity( sFrobber );

		TryToggle( pFrobber );
	}
	else if( EventName == sOnInitialOrientationSet )
	{
		AdjustForFacing();
		UpdateFromOpenState( true, false );
	}
	else if( EventName == sOnLoaded )
	{
		UpdateFromOpenState( true, true );
	}
	else if( EventName == sLock )
	{
		Lock();
	}
	else if( EventName == sUnlock )
	{
		Unlock();
	}
}

/*virtual*/ void WBCompEldDoor::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Bool, IsDoor, true );		// To distinguish from e.g. EldLock component
	WB_SET_CONTEXT( Event, Bool, Open, m_Open );
	WB_SET_CONTEXT( Event, Bool, Locked, m_Locked );
}

/*virtual*/ void WBCompEldDoor::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	WBEntity* const				pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();
	WBCompEldMesh* const		pMesh		= GET_WBCOMP( pEntity, EldMesh );

	DEVASSERT( pTransform );
	DEVASSERT( pMesh );

	m_OffsetInterpolator.Tick( DeltaTime );
	m_OrientationInterpolator.Tick( DeltaTime );

	pMesh->SetMeshOffset( m_OffsetInterpolator.GetValue() );
	pTransform->SetOrientation( m_OrientationInterpolator.GetValue() );
}

bool WBCompEldDoor::CanOpenDoor( WBEntity* const pFrobber )
{
	if( !pFrobber )
	{
		return true;
	}

	// Well, this is an abuse of the system. But it makes it easy to deal
	// with AI motion, and I can hack things further for the player.
	WBEvent ContextEvent;
	pFrobber->AddContextToEvent( ContextEvent );

	STATIC_HASHED_STRING( CanOpenDoors );
	const bool FrobberCanOpenDoors = ContextEvent.GetBool( sCanOpenDoors );

	STATIC_HASHED_STRING( CanUnlockDoors );
	const bool FrobberCanUnlockDoors = ContextEvent.GetBool( sCanUnlockDoors );

	STATIC_HASHED_STRING( Keys );
	const uint Keys = ContextEvent.GetInt( sKeys );

	if( !FrobberCanOpenDoors )
	{
		return false;
	}

	if( !m_Locked )
	{
		return true;
	}

	if( FrobberCanUnlockDoors )
	{
		return true;
	}

	if( Keys > 0 )
	{
		// Notify frobber that a key was used
		WB_MAKE_EVENT( OnKeyUsed, pFrobber );
		WB_DISPATCH_EVENT( GetEventManager(), OnKeyUsed, pFrobber );

		// And unlock the door!
		Unlock();

		return true;
	}

	return false;
}

void WBCompEldDoor::TryToggle( WBEntity* const pFrobber )
{
	if( m_Open )
	{
		EldritchWorld* const pWorld = GetWorld();
		DEVASSERT( pWorld );

		WBEntity* const pEntity = GetEntity();
		DEVASSERT( pEntity );

		WBCompEldTransform* const pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();
		WBCompEldCollision* const pCollision	= GET_WBCOMP( pEntity, EldCollision );
		DEVASSERT( pTransform );
		DEVASSERT( pCollision );

		CollisionInfo Info;
		Info.m_CollideEntities = true;
		Info.m_CollidingEntity = pEntity;
		Info.m_UserFlags = EECF_CollideAsWorld | EECF_CollideStaticEntities | EECF_CollideDynamicEntities;

		if( pWorld->CheckClearance( pTransform->GetLocation(), pCollision->GetExtents(), Info ) )
		{
			// Door is blocked from closing
		}
		else
		{
			Toggle();
		}
	}
	else
	{
		if( !CanOpenDoor( pFrobber ) )
		{
			WB_MAKE_EVENT( TryToggleFailed, GetEntity() );
			WB_DISPATCH_EVENT( GetEventManager(), TryToggleFailed, GetEntity() );
		}
		else
		{
			Toggle();

			WB_MAKE_EVENT( TryToggleSucceeded, GetEntity() );
			WB_DISPATCH_EVENT( GetEventManager(), TryToggleSucceeded, GetEntity() );
		}
	}
}

void WBCompEldDoor::Toggle()
{
	m_Open = !m_Open;
	UpdateFromOpenState( false, false );

	if( m_Open )
	{
		WB_MAKE_EVENT( OnOpened, GetEntity() );
		WB_DISPATCH_EVENT( GetEventManager(), OnOpened, GetEntity() );
	}
	else
	{
		WB_MAKE_EVENT( OnClosed, GetEntity() );
		WB_DISPATCH_EVENT( GetEventManager(), OnClosed, GetEntity() );
	}
}

void WBCompEldDoor::Lock()
{
	WBEntity* const				pEntity		= GetEntity();
	WBCompEldCollision* const	pCollision	= GET_WBCOMP( pEntity, EldCollision );
	DEVASSERT( pCollision );

	// If the door is closed, clear current nav state since we'll be adding the locked flag
	if( !m_Open )
	{
		EldritchNav::GetInstance()->UpdateWorldFromEntity( pCollision, false );
	}

	m_Locked = true;

	if( m_LockedMesh != "" )
	{
		WB_MAKE_EVENT( SetMesh, GetEntity() );
		WB_SET_AUTO( SetMesh, Hash, Mesh, m_LockedMesh );
		WB_DISPATCH_EVENT( GetEventManager(), SetMesh, GetEntity() );
	}

	if( m_LockedTexture != "" )
	{
		WB_MAKE_EVENT( SetTexture, GetEntity() );
		WB_SET_AUTO( SetTexture, Hash, Texture, m_LockedTexture );
		WB_DISPATCH_EVENT( GetEventManager(), SetTexture, GetEntity() );
	}

	if( m_LockedFriendlyName != "" )
	{
		WB_MAKE_EVENT( SetFriendlyName, GetEntity() );
		WB_SET_AUTO( SetFriendlyName, Hash, FriendlyName, m_LockedFriendlyName );
		WB_DISPATCH_EVENT( GetEventManager(), SetFriendlyName, GetEntity() );
	}

	// If the door is closed, update nav state to add the locked flag
	if( !m_Open )
	{
		EldritchNav::GetInstance()->UpdateWorldFromEntity( pCollision, true );
	}

	WB_MAKE_EVENT( OnLocked, GetEntity() );
	WB_DISPATCH_EVENT( GetEventManager(), OnLocked, GetEntity() );
}

void WBCompEldDoor::Unlock()
{
	WBEntity* const				pEntity		= GetEntity();
	WBCompEldCollision* const	pCollision	= GET_WBCOMP( pEntity, EldCollision );
	DEVASSERT( pCollision );

	// If the door is closed, clear current nav state since we'll be removing the locked flag
	if( !m_Open )
	{
		EldritchNav::GetInstance()->UpdateWorldFromEntity( pCollision, false );
	}

	m_Locked = false;

	if( m_UnlockedMesh != "" )
	{
		WB_MAKE_EVENT( SetMesh, GetEntity() );
		WB_SET_AUTO( SetMesh, Hash, Mesh, m_UnlockedMesh );
		WB_DISPATCH_EVENT( GetEventManager(), SetMesh, GetEntity() );
	}

	if( m_UnlockedTexture != "" )
	{
		WB_MAKE_EVENT( SetTexture, GetEntity() );
		WB_SET_AUTO( SetTexture, Hash, Texture, m_UnlockedTexture );
		WB_DISPATCH_EVENT( GetEventManager(), SetTexture, GetEntity() );
	}

	if( m_UnlockedFriendlyName != "" )
	{
		WB_MAKE_EVENT( SetFriendlyName, GetEntity() );
		WB_SET_AUTO( SetFriendlyName, Hash, FriendlyName, m_UnlockedFriendlyName );
		WB_DISPATCH_EVENT( GetEventManager(), SetFriendlyName, GetEntity() );
	}

	// If the door is closed, update nav state to remove the locked flag
	if( !m_Open )
	{
		EldritchNav::GetInstance()->UpdateWorldFromEntity( pCollision, true );
	}

	WB_MAKE_EVENT( OnUnlocked, GetEntity() );
	WB_DISPATCH_EVENT( GetEventManager(), OnUnlocked, GetEntity() );
}

void WBCompEldDoor::UpdateFromOpenState( const bool InitialSetup, const bool SuppressWorldChange )
{
	EldritchWorld* const		pWorld		= GetWorld();
	DEVASSERT( pWorld );

	WBEntity* const				pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldMesh* const		pMesh		= GET_WBCOMP( pEntity, EldMesh );
	WBCompEldCollision* const	pCollision	= GET_WBCOMP( pEntity, EldCollision );
	WBCompEldFrobbable* const	pFrobbable	= GET_WBCOMP( pEntity, EldFrobbable );

	DEVASSERT( pMesh );
	DEVASSERT( pCollision );
	DEVASSERT( pFrobbable );

	const float CurrentInterpT		= m_OffsetInterpolator.GetT();
	const float InterpTime			= InitialSetup ? 0.0f : ( m_InterpTime * CurrentInterpT );

	const uint	OcclusionFlag		= pCollision->GetDefaultCollisionFlags() & EECF_BlocksOcclusion;
	const uint	DoorCollisionMask	= OcclusionFlag | EECF_BlocksEntities | EECF_BlocksTrace;

	if( m_Open )
	{
		m_OffsetInterpolator.Reset( Interpolator<Vector>::EIT_Linear, m_OffsetInterpolator.GetValue(), m_OpenOffset, InterpTime );
		m_OrientationInterpolator.Reset( Interpolator<Angles>::EIT_Linear, m_OrientationInterpolator.GetValue(), m_OpenOrientation, InterpTime );

		pCollision->SetCollisionFlags( 0, DoorCollisionMask, true );
		pFrobbable->SetBoundOffset( m_OpenFrobOffset );
		pFrobbable->SetBoundExtents( m_OpenFrobExtents );
		pMesh->SetBlendedIrradianceOffset( m_OpenIrradianceOffset );

		if( SuppressWorldChange )
		{
			// We're loading or something, no need to notify world
		}
		else
		{
			pWorld->OnBoxChanged( pCollision->GetBounds() );
		}

		if( InitialSetup )
		{
			// Do nothing, a serialized open door should already be removed from the nav world.
		}
		else
		{
			EldritchNav::GetInstance()->UpdateWorldFromEntity( pCollision, false );
		}
	}
	else
	{
		m_OffsetInterpolator.Reset( Interpolator<Vector>::EIT_Linear, m_OffsetInterpolator.GetValue(), m_ClosedOffset, InterpTime );
		m_OrientationInterpolator.Reset( Interpolator<Angles>::EIT_Linear, m_OrientationInterpolator.GetValue(), m_ClosedOrientation, InterpTime );

		pCollision->SetCollisionFlags( DoorCollisionMask, DoorCollisionMask, true );
		pFrobbable->SetBoundOffset( m_ClosedFrobOffset );
		pFrobbable->SetBoundExtents( m_ClosedFrobExtents );
		pMesh->SetBlendedIrradianceOffset( m_ClosedIrradianceOffset );

		if( SuppressWorldChange )
		{
			// We're loading or something, no need to notify world
		}
		else
		{
			pWorld->OnBoxChanged( pCollision->GetBounds() );
		}

		if( InitialSetup )
		{
			// HACK: Don't double up where EldCollision already updates the nav world from this entity.
			// TODO: I might need to do this for OnBoxChanged, too; it's possible that those problems (false
			// shadows, etc.) are just hidden by virtue of a whole mesh getting updated when the box changes.
			// If I ever see inexplicable shadows near doors, save the random seed and try that.
		}
		else
		{
			EldritchNav::GetInstance()->UpdateWorldFromEntity( pCollision, true );
		}
	}
}

#define VERSION_EMPTY			0
#define VERSION_OPEN			1
#define VERSION_FACINGCONFIG	2
#define VERSION_LOCKED			3
#define VERSION_CURRENT			3

uint WBCompEldDoor::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;	// Version
	Size += 1;	// m_Open

	Size += sizeof( Vector );	// m_ClosedOffset
	Size += 4;					// m_ClosedOrientation.Yaw
	Size += sizeof( Vector );	// m_ClosedFrobOffset
	Size += sizeof( Vector );	// m_ClosedFrobExtents
	Size += sizeof( Vector );	// m_ClosedIrradianceOffset
	Size += sizeof( Vector );	// m_OpenOffset
	Size += 4;					// m_OpenOrientation.Yaw
	Size += sizeof( Vector );	// m_OpenFrobOffset
	Size += sizeof( Vector );	// m_OpenFrobExtents
	Size += sizeof( Vector );	// m_OpenIrradianceOffset

	Size += 1;					// m_Locked

	return Size;
}

void WBCompEldDoor::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteBool( m_Open );

	Stream.Write( sizeof( Vector ), &m_ClosedOffset );
	Stream.WriteFloat( m_ClosedOrientation.Yaw );
	Stream.Write( sizeof( Vector ), &m_ClosedFrobOffset );
	Stream.Write( sizeof( Vector ), &m_ClosedFrobExtents );
	Stream.Write( sizeof( Vector ), &m_ClosedIrradianceOffset );
	Stream.Write( sizeof( Vector ), &m_OpenOffset );
	Stream.WriteFloat( m_OpenOrientation.Yaw );
	Stream.Write( sizeof( Vector ), &m_OpenFrobOffset );
	Stream.Write( sizeof( Vector ), &m_OpenFrobExtents );
	Stream.Write( sizeof( Vector ), &m_OpenIrradianceOffset );

	Stream.WriteBool( m_Locked );
}

void WBCompEldDoor::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_OPEN )
	{
		m_Open = Stream.ReadBool();
	}

	if( Version >= VERSION_FACINGCONFIG )
	{
		Stream.Read( sizeof( Vector ), &m_ClosedOffset );
		m_ClosedOrientation.Yaw = Stream.ReadFloat();
		Stream.Read( sizeof( Vector ), &m_ClosedFrobOffset );
		Stream.Read( sizeof( Vector ), &m_ClosedFrobExtents );
		Stream.Read( sizeof( Vector ), &m_ClosedIrradianceOffset );
		Stream.Read( sizeof( Vector ), &m_OpenOffset );
		m_OpenOrientation.Yaw = Stream.ReadFloat();
		Stream.Read( sizeof( Vector ), &m_OpenFrobOffset );
		Stream.Read( sizeof( Vector ), &m_OpenFrobExtents );
		Stream.Read( sizeof( Vector ), &m_OpenIrradianceOffset );
	}

	if( Version >= VERSION_LOCKED )
	{
		m_Locked = Stream.ReadBool();
	}
}