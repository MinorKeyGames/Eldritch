#include "core.h"
#include "wbcompelditem.h"
#include "configmanager.h"
#include "wbcompeldtransform.h"
#include "Components/wbcompowner.h"
#include "Components/wbcompeldmesh.h"
#include "Components/wbcompeldcollision.h"
#include "mathcore.h"
#include "wbeventmanager.h"
#include "idatastream.h"
#include "eldritchworld.h"
#include "collisioninfo.h"

WBCompEldItem::WBCompEldItem()
:	m_Slot()
,	m_DropSpawn( "" )
,	m_DropSpawnImpulse( 0.0f )
,	m_DropSpawnImpulseZ( 0.0f )
,	m_DropSpawnOffsetZ( 0.0f )
,	m_DropSpawnYaw( 0.0f )
,	m_SuppressDropDupe( false )
{
}

WBCompEldItem::~WBCompEldItem()
{
}

/*virtual*/ void WBCompEldItem::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Slot );
	m_Slot = ConfigManager::GetInheritedHash( sSlot, HashedString::NullString, sDefinitionName );

	STATICHASH( DropSpawn );
	m_DropSpawn = ConfigManager::GetInheritedString( sDropSpawn, "", sDefinitionName );

	STATICHASH( DropSpawnImpulse );
	m_DropSpawnImpulse = ConfigManager::GetInheritedFloat( sDropSpawnImpulse, 0.0f, sDefinitionName );

	STATICHASH( DropSpawnImpulseZ );
	m_DropSpawnImpulseZ = ConfigManager::GetInheritedFloat( sDropSpawnImpulseZ, 0.0f, sDefinitionName );

	STATICHASH( DropSpawnOffsetZ );
	m_DropSpawnOffsetZ = ConfigManager::GetInheritedFloat( sDropSpawnOffsetZ, 0.0f, sDefinitionName );

	STATICHASH( DropSpawnYaw );
	m_DropSpawnYaw = DEGREES_TO_RADIANS( ConfigManager::GetInheritedFloat( sDropSpawnYaw, 0.0f, sDefinitionName ) );

	STATICHASH( SuppressDropDupe );
	m_SuppressDropDupe = ConfigManager::GetInheritedBool( sSuppressDropDupe, false, sDefinitionName );
}

/*virtual*/ void WBCompEldItem::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnUnequipped );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnUnequipped )
	{
		STATIC_HASHED_STRING( SuppressSpawn );
		const bool SuppressSpawn = Event.GetBool( sSuppressSpawn );

		STATIC_HASHED_STRING( ReplacingItem );
		WBEntity* const pReplacingItem = Event.GetEntity( sReplacingItem );

		// Crude HACK: Suppress spawn if this item is hidden, because it's already been thrown.
		WBCompEldMesh* const pMesh = GET_WBCOMP( GetEntity(), EldMesh );
		const bool MeshHidden = pMesh && pMesh->IsHidden();

		if( SuppressSpawn ||
			MeshHidden ||
			( m_SuppressDropDupe && pReplacingItem && pReplacingItem->GetName() == GetEntity()->GetName() ) )
		{
			// Do nothing!
		}
		else
		{
			// Spawn a pickup for the dropped item
			SpawnDrop();
		}
	}
}

void WBCompEldItem::SpawnDrop() const
{
	if( m_DropSpawn == "" )
	{
		return;
	}

	Vector SpawnLocation;
	Vector SpawnImpulse;
	Angles SpawnOrientation;
	if( !GetSpawnDropTransform( SpawnLocation, SpawnImpulse, SpawnOrientation ) )
	{
		// Nowhere to spawn
		return;
	}

	WBEntity* const				pDropEntity		= WBWorld::GetInstance()->CreateEntity( m_DropSpawn );
	WBCompEldTransform* const	pDropTransform	= pDropEntity->GetTransformComponent<WBCompEldTransform>();
	ASSERT( pDropTransform );

	WBCompOwner* const			pOwnerComponent	= GET_WBCOMP( GetEntity(), Owner );
	WBCompOwner* const			pDropOwner		= GET_WBCOMP( pDropEntity, Owner );

	if( pOwnerComponent && pDropOwner )
	{
		pDropOwner->SetOwner( pOwnerComponent->GetOwner() );
	}

	WBCompEldCollision* const	pDropCollision	= GET_WBCOMP( pDropEntity, EldCollision );
	if( pDropCollision )
	{
		CollisionInfo Info;
		Info.m_CollideWorld		= true;
		Info.m_CollideEntities	= true;
		Info.m_CollidingEntity	= GetEntity();	// Using the owner, not the spawned entity (which should be at origin at the moment)
		Info.m_UserFlags		= EECF_EntityCollision;

		GetWorld()->FindSpot( SpawnLocation, pDropCollision->GetExtents(), Info );
	}

	pDropTransform->SetLocation( SpawnLocation );
	pDropTransform->SetOrientation( SpawnOrientation );
	pDropTransform->ApplyImpulse( SpawnImpulse );

	WB_MAKE_EVENT( OnInitialOrientationSet, pDropEntity );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), OnInitialOrientationSet, pDropEntity );
}

bool WBCompEldItem::GetSpawnDropTransform( Vector& OutLocation, Vector& OutImpulse, Angles& OutOrientation ) const
{
	WBCompEldTransform* pSelectedTransform = NULL;

	// First, try to spawn from our owner's transform.
	WBCompOwner* const	pOwnerComponent	= GET_WBCOMP( GetEntity(), Owner );
	if( pOwnerComponent )
	{
		WBEntity* const	pOwnerEntity	= pOwnerComponent->GetOwner();
		if( pOwnerEntity )
		{
			pSelectedTransform			= pOwnerEntity->GetTransformComponent<WBCompEldTransform>();
		}
	}

	// If we don't have an owner, try to spawn from our own transform.
	if( !pSelectedTransform )
	{
		WBEntity* const	pEntity	= GetEntity();
		DEVASSERT( pEntity );

		pSelectedTransform		= pEntity->GetTransformComponent<WBCompEldTransform>();
	}

	if( pSelectedTransform )
	{
		OutLocation				= pSelectedTransform->GetLocation();
		OutLocation.z			+= m_DropSpawnOffsetZ;

		OutOrientation			= pSelectedTransform->GetOrientation();
		OutOrientation.Pitch	= 0.0f;
		OutOrientation.Yaw		+= m_DropSpawnYaw;

		OutImpulse				= pSelectedTransform->GetOrientation().ToVector();
		OutImpulse.z			+= m_DropSpawnImpulseZ;
		OutImpulse.FastNormalize();
		OutImpulse				*= m_DropSpawnImpulse;

		return true;
	}

	// Else, we don't have anywhere to spawn from.
	return false;
}

#define VERSION_EMPTY	0
#define VERSION_SLOT	1
#define VERSION_CURRENT	1

uint WBCompEldItem::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;						// Version
	Size += sizeof( HashedString );	// m_Slot

	return Size;
}

void WBCompEldItem::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteHashedString( m_Slot );
}

void WBCompEldItem::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_SLOT )
	{
		m_Slot = Stream.ReadHashedString();
	}
}