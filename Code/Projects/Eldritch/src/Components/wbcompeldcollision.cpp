#include "core.h"
#include "wbcompeldcollision.h"
#include "collisioninfo.h"
#include "segment.h"
#include "eldritchworld.h"
#include "configmanager.h"
#include "wbcompeldtransform.h"
#include "wbentity.h"
#include "clock.h"
#include "eldritchframework.h"
#include "irenderer.h"
#include "array.h"
#include "aabb.h"
#include "wbscene.h"
#include "wbeventmanager.h"
#include "idatastream.h"
#include "mathcore.h"
#include "eldritchnav.h"

/*static*/ WBCompEldCollision::TCollisionMap	WBCompEldCollision::sm_CollisionMap;
/*static*/ WBCompEldCollision::TCollisionArray	WBCompEldCollision::sm_TouchingArray;

WBCompEldCollision::WBCompEldCollision()
:	m_HalfExtents()
,	m_Bounds()
,	m_Elasticity( 0.0f )
,	m_FrictionTargetTime( 0.0f )
,	m_FrictionTargetPercentVelocity( 0.0f )
,	m_IsNavBlocking( false )
,	m_Landed( false )
,	m_UnlandedTime( 0.0f )
,	m_Touching()
,	m_CanTouch( false )
,	m_CollisionFlags( 0 )
,	m_DefaultCollisionFlags( 0 )
{
}

WBCompEldCollision::~WBCompEldCollision()
{
	RemoveFromCollisionMap();
	RemoveFromTouchingArray();
}

/*virtual*/ void WBCompEldCollision::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( EldCollision );

	// "Static" is a poor name, but these entities will be included in collision (not touching)
	// tests, occlusion tests, and nav tests. See EEldritchCollisionFlags for exact definition.
	STATICHASH( IsStatic );
	const bool IsStatic = ConfigManager::GetInheritedBool( sIsStatic, false, sDefinitionName );
	SetCollisionFlags( IsStatic ? EECF_IsStatic : EECF_IsDynamic, EECF_Mask_EntityTypes, false, false );

	STATICHASH( CanTouch );
	m_CanTouch = ConfigManager::GetInheritedBool( sCanTouch, !IsStatic, sDefinitionName );

	STATICHASH( HalfExtentsXY );
	const float HalfExtentsXY = ConfigManager::GetInheritedFloat( sHalfExtentsXY, 0.0f, sDefinitionName );

	STATICHASH( HalfExtentsX );
	const float HalfExtentsX = ConfigManager::GetInheritedFloat( sHalfExtentsX, HalfExtentsXY, sDefinitionName );

	STATICHASH( HalfExtentsY );
	const float HalfExtentsY = ConfigManager::GetInheritedFloat( sHalfExtentsY, HalfExtentsXY, sDefinitionName );

	STATICHASH( HalfExtentsZ );
	const float HalfExtentsZ = ConfigManager::GetInheritedFloat( sHalfExtentsZ, 0.0f, sDefinitionName );

	m_HalfExtents = Vector( HalfExtentsX, HalfExtentsY, HalfExtentsZ );

	STATICHASH( Elasticity );
	m_Elasticity = ConfigManager::GetInheritedFloat( sElasticity, 0.0f, sDefinitionName );

	STATICHASH( FrictionTargetTime );
	const float DefaultFrictionTargetTime = ConfigManager::GetFloat( sFrictionTargetTime, 0.0f, sEldCollision );
	m_FrictionTargetTime = ConfigManager::GetInheritedFloat( sFrictionTargetTime, DefaultFrictionTargetTime, sDefinitionName );
	ASSERT( m_FrictionTargetTime > 0.0f );

	STATICHASH( FrictionTargetPercentVelocity );
	const float DefaultFrictionTargetPercentVelocity = ConfigManager::GetFloat( sFrictionTargetPercentVelocity, 0.0f, sEldCollision );
	m_FrictionTargetPercentVelocity = ConfigManager::GetInheritedFloat( sFrictionTargetPercentVelocity, DefaultFrictionTargetPercentVelocity, sDefinitionName );

	STATICHASH( BlocksWorld );
	const bool BlocksWorld = ConfigManager::GetInheritedBool( sBlocksWorld, true, sDefinitionName );
	SetCollisionFlags( BlocksWorld ? EECF_BlocksWorld : EECF_None, EECF_BlocksWorld, false, false );

	STATICHASH( BlocksEntities );
	const bool BlocksEntities = ConfigManager::GetInheritedBool( sBlocksEntities, false, sDefinitionName );
	SetCollisionFlags( BlocksEntities ? EECF_BlocksEntities : EECF_None, EECF_BlocksEntities, false, false );

	STATICHASH( BlocksOcclusion );
	const bool BlocksOcclusion = ConfigManager::GetInheritedBool( sBlocksOcclusion, false, sDefinitionName );
	SetCollisionFlags( BlocksOcclusion ? EECF_BlocksOcclusion : EECF_None, EECF_BlocksOcclusion, false, false );

	STATICHASH( BlocksTrace );
	const bool BlocksTrace = ConfigManager::GetInheritedBool( sBlocksTrace, false, sDefinitionName );
	SetCollisionFlags( BlocksTrace ? EECF_BlocksTrace : EECF_None, EECF_BlocksTrace, false, false );

	m_DefaultCollisionFlags = m_CollisionFlags;

	AddToCollisionMap();
	AddToTouchingArray();
}

void WBCompEldCollision::SetCollisionFlags( const uint Flags, const uint Mask /*= 0xffffffff*/, const bool SendEvents /*= false*/, const bool UpdateCollisionMap /*= true*/ )
{
	XTRACE_FUNCTION;

	// Make sure the mask includes all the bits in Flags
	ASSERT( CountBits( Mask ) >= CountBits( Flags | Mask ) );

	if( UpdateCollisionMap )
	{
		RemoveFromCollisionMap();
	}

	// Lower all the bits in the mask
	m_CollisionFlags &= ~Mask;

	// Raise all the bits in the flag and mask
	m_CollisionFlags |= Flags & Mask;

	if( UpdateCollisionMap )
	{
		AddToCollisionMap();
	}

	if( SendEvents )
	{
		ConditionalSendStaticCollisionChangedEvent();
	}
}

float WBCompEldCollision::GetFrictionCoefficient() const
{
	if( m_FrictionTargetPercentVelocity >= 1.0f )
	{
		return 1.0f;
	}

	// Relationship between friction and the velocity we want to be reduced to at time t:
	//	TargetPercentVelocity = Pow( Mu, TargetTime / DeltaTime )
	//	Mu = Pow( TargetPercentVelocity, DeltaTime / TargetTime )
	const float DeltaTime = GetFramework()->GetClock()->GetGameDeltaTime();
	const float Mu = Pow( m_FrictionTargetPercentVelocity, DeltaTime / m_FrictionTargetTime );	// Friction coefficient
	return Mu;
}

void WBCompEldCollision::Collide( const Vector& StartLocation, Vector& InOutMovement )
{
	XTRACE_FUNCTION;

	static const Vector UpVector( 0.0f, 0.0f, 1.0f );

	WBCompEldTransform* pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );	// We shouldn't have a collision without a transform

	if( InOutMovement.Dot( UpVector ) < 0.0f )
	{
		// We're falling.
		Fall();
	}

	bool Collision;
	do
	{
		Collision = false;

		// Collision detection
		CollisionInfo Info;
		Info.m_CollideWorld = true;
		Info.m_CollideEntities = true;
		Info.m_CollidingEntity = GetEntity();
		Info.m_UserFlags = EECF_EntityCollision;

		Segment SweepSegment( StartLocation, StartLocation + InOutMovement );
		if( GetWorld()->Sweep( SweepSegment, m_HalfExtents, Info ) )
		{
			// Collision response
			// TODO: Apply friction, reflection, zero out velocity on transform component, whatever.

			Collision = true;

			if( Info.m_Plane.m_Normal.LengthSquared() > 0.0f )
			{
				InOutMovement = Info.m_Plane.ProjectVector( InOutMovement );

				const bool LandedOnFloor = ( Info.m_Plane.m_Normal.Dot( UpVector ) > 0.9f );
				const float Friction = LandedOnFloor ? GetFrictionCoefficient() : 1.0f;

				// TODO: Handle friction different for different surface normals
				// (because you don't want to grip walls/ceilings the way you do floors).
				// Or maybe a factor of the entity's velocity on the normal? Is that how friction works?
				const Vector OldVelocity = pTransform->GetVelocity();
				const Vector VelocityInCollisionPlane = Info.m_Plane.ProjectVector( OldVelocity );
				const Vector OrthogonalVelocityComponent = VelocityInCollisionPlane * Friction;

				const Vector VelocityInCollisionNormal = OldVelocity.ProjectionOnto( Info.m_Plane.m_Normal );
				const Vector ParallelVelocityComponent = VelocityInCollisionNormal * -m_Elasticity;

				const Vector NewVelocity = OrthogonalVelocityComponent + ParallelVelocityComponent;
				pTransform->SetVelocity( NewVelocity );

				WBEntity* const pHitEntity = static_cast<WBEntity*>( Info.m_HitEntity );

				if( LandedOnFloor )
				{
					const float LandedMagnitude = -VelocityInCollisionNormal.z;
					ASSERT( LandedMagnitude >= 0.0f );
					OnLanded( LandedMagnitude, pHitEntity );
				}
				else
				{
					OnCollided( Info.m_Plane.m_Normal, pHitEntity );
				}
			}
			else
			{
				// Complete collision, probably started the move inside geometry.
				InOutMovement = Vector();

				// This was commented out for a long time... but I can't see any reason not to do it,
				// and it fixes things acquiring a huge downward velocity from gravity while they're stuck.
				pTransform->SetVelocity( Vector() );
			}
		}
	}
	while( InOutMovement.LengthSquared() > 0.0f && Collision );
}

void WBCompEldCollision::OnLanded( const float LandedMagnitude, WBEntity* const pCollidedEntity )
{
	XTRACE_FUNCTION;

	if( !IsRecentlyLanded( 0.0f ) )
	{
		WB_MAKE_EVENT( OnLanded, GetEntity() );
		WB_SET_AUTO( OnLanded, Float, LandedMagnitude, LandedMagnitude );
		WB_DISPATCH_EVENT( GetEventManager(), OnLanded, GetEntity() );

		// OnAnyCollision is fired from both OnLanded and OnCollided.
		WB_MAKE_EVENT( OnAnyCollision, GetEntity() );
		WB_DISPATCH_EVENT( GetEventManager(), OnAnyCollision, GetEntity() );
	}

	// Notify the collided object too (regardless of this entity's landed state)
	if( pCollidedEntity )
	{
		WB_MAKE_EVENT( OnAnyCollision, pCollidedEntity );
		WB_DISPATCH_EVENT( GetEventManager(), OnAnyCollision, pCollidedEntity );
	}

	m_Landed = true;
}

void WBCompEldCollision::OnCollided( const Vector& CollisionNormal, WBEntity* const pCollidedEntity )
{
	XTRACE_FUNCTION;

	// OnCollided actually means *on collided with non-ground surface*, as distinguished from OnLanded.
	WB_MAKE_EVENT( OnCollided, GetEntity() );
	WB_SET_AUTO( OnCollided, Vector, CollisionNormal, CollisionNormal );
	WB_SET_AUTO( OnCollided, Entity, CollidedEntity, pCollidedEntity );
	WB_DISPATCH_EVENT( GetEventManager(), OnCollided, GetEntity() );

	// OnAnyCollision handles both.
	WB_MAKE_EVENT( OnAnyCollision, GetEntity() );
	WB_DISPATCH_EVENT( GetEventManager(), OnAnyCollision, GetEntity() );

	// Notify the collided object too.
	if( pCollidedEntity )
	{
		WB_MAKE_EVENT( OnAnyCollision, pCollidedEntity );
		WB_DISPATCH_EVENT( GetEventManager(), OnAnyCollision, pCollidedEntity );
	}
}

void WBCompEldCollision::Jump()
{
	if( m_Landed )
	{
		m_Landed = false;
		m_UnlandedTime = 0.0f;
	}
}

void WBCompEldCollision::Fall()
{
	if( m_Landed )
	{
		m_Landed = false;
		m_UnlandedTime = GetFramework()->GetClock()->GetGameCurrentTime();
	}
}

bool WBCompEldCollision::IsRecentlyLanded( const float TimeThreshold ) const
{
	if( m_Landed )
	{
		return true;
	}
	else
	{
		const float CurrentTime = GetFramework()->GetClock()->GetGameCurrentTime();
		const float TimeSinceUnlanded = CurrentTime - m_UnlandedTime;
		return ( TimeSinceUnlanded <= TimeThreshold );
	}
}

AABB WBCompEldCollision::GetCurrentBounds() const
{
	WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	return AABB::CreateFromCenterAndExtents( pTransform->GetLocation(), m_HalfExtents );
}

#if BUILD_DEV
/*virtual*/ void WBCompEldCollision::DebugRender() const
{
	WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const Vector Location = pTransform->GetLocation();
	GetFramework()->GetRenderer()->DEBUGDrawBox( Location - m_HalfExtents, Location + m_HalfExtents, ARGB_TO_COLOR( 255, 255, 255, 255 ) );
}
#endif

/*virtual*/ void WBCompEldCollision::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnJumped );
	STATIC_HASHED_STRING( OnMoved );
	STATIC_HASHED_STRING( OnInitialOrientationSet );
	STATIC_HASHED_STRING( OnLoaded );
	STATIC_HASHED_STRING( OnDestroyed );
	STATIC_HASHED_STRING( StopTouching );
	STATIC_HASHED_STRING( StartTouching );
	STATIC_HASHED_STRING( StopBlockingWorld );
	STATIC_HASHED_STRING( SetDefaultFriction );
	STATIC_HASHED_STRING( DisableCollision );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnJumped )
	{
		Jump();
	}
	else if( EventName == sOnLoaded )
	{
		UpdateBounds();

		// If this entity is a nav blocker, add its bounds during loading.
		ConditionalSetNavBlocking( true );
	}
	else if( EventName == sOnMoved )
	{
		// Remove nav blocker at old location
		ConditionalSetNavBlocking( false );

		// Change box at old location
		if( MatchesAllCollisionFlags( EECF_Occlusion ) )
		{
			GetWorld()->OnBoxChanged( GetBounds() );
		}

		UpdateBounds();
		UpdateTouching();

		if( MatchesAllCollisionFlags( EECF_Occlusion ) )
		{
			GetWorld()->OnBoxChanged( GetBounds() );
		}

		// Add nav blocker at new location
		ConditionalSetNavBlocking( true );
		ConditionalSendStaticCollisionChangedEvent();
	}
	else if( EventName == sOnInitialOrientationSet )
	{
		// For initialization of certain classes of static entities, fix up extents based on orientation.
		if( m_HalfExtents.x != m_HalfExtents.y )
		{
			STATIC_HASHED_STRING( Orientation );
			const Angles Orientation = Event.GetAngles( sOrientation );

			if( Equal( Orientation.Yaw, DEGREES_TO_RADIANS( 90.0f ) ) ||
				Equal( Orientation.Yaw, DEGREES_TO_RADIANS( 270.0f ) ) )
			{
				// Remove nav blocker at old location
				ConditionalSetNavBlocking( false );

				Swap( m_HalfExtents.x, m_HalfExtents.y );

				UpdateBounds();
				UpdateTouching();

				if( MatchesAllCollisionFlags( EECF_Occlusion ) )
				{
					GetWorld()->OnBoxChanged( GetBounds() );
				}

				// Add nav blocker at new orientation
				ConditionalSetNavBlocking( true );
			}
		}
	}
	else if( EventName == sOnDestroyed )
	{
		UpdateTouching();

		// If this entity is a nav blocker, remove its bounds when it is destroyed.
		ConditionalSetNavBlocking( false );

		if( MatchesAllCollisionFlags( EECF_Occlusion ) )
		{
			GetWorld()->OnBoxChanged( GetBounds() );
		}

		ConditionalSendStaticCollisionChangedEvent();
	}
	else if( EventName == sStopTouching )
	{
		if( m_CanTouch )
		{
			RemoveFromTouchingArray();

			m_CanTouch = false;
			SetCollisionFlags( EECF_None, EECF_BlocksTrace );

			UpdateTouching();
		}
	}
	else if( EventName == sStartTouching )
	{
		if( !m_CanTouch )
		{
			m_CanTouch = true;
			SetCollisionFlags( m_DefaultCollisionFlags & EECF_BlocksTrace, EECF_BlocksTrace );

			AddToTouchingArray();
			UpdateTouching();
		}
	}
	else if( EventName == sStopBlockingWorld )
	{
		SetCollisionFlags( EECF_None, EECF_BlocksWorld );
	}
	else if( EventName == sDisableCollision )
	{
		SetCollisionFlags( EECF_None );
	}
	else if( EventName == sSetDefaultFriction )
	{
		STATICHASH( EldCollision );

		STATICHASH( FrictionTargetTime );
		const float DefaultFrictionTargetTime = ConfigManager::GetFloat( sFrictionTargetTime, 0.0f, sEldCollision );

		STATICHASH( FrictionTargetPercentVelocity );
		const float DefaultFrictionTargetPercentVelocity = ConfigManager::GetFloat( sFrictionTargetPercentVelocity, 0.0f, sEldCollision );

		m_FrictionTargetTime = DefaultFrictionTargetTime;
		ASSERT( m_FrictionTargetTime > 0.0f );

		m_FrictionTargetPercentVelocity = DefaultFrictionTargetPercentVelocity;
	}
}

/*virtual*/ void WBCompEldCollision::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Bool, IsLanded, IsRecentlyLanded( 0.0f ) );
	WB_SET_CONTEXT( Event, Bool, IsStatic, MatchesAllCollisionFlags( EECF_IsStatic ) );
}

void WBCompEldCollision::ConditionalSetNavBlocking( const bool NavBlocking )
{
	if( m_IsNavBlocking == NavBlocking )
	{
		return;
	}

	if( MatchesAllCollisionFlags( EECF_Nav ) )
	{
		EldritchNav::GetInstance()->UpdateWorldFromEntity( this, NavBlocking );
		m_IsNavBlocking = NavBlocking;
	}
}

void WBCompEldCollision::ConditionalSendStaticCollisionChangedEvent()
{
	XTRACE_FUNCTION;

	if( MatchesAllCollisionFlags( EECF_IsStatic ) )
	{
		WB_MAKE_EVENT( OnStaticCollisionChanged, GetEntity() );
		WB_DISPATCH_EVENT( GetEventManager(), OnStaticCollisionChanged, GetEntity() );
	}
}

void WBCompEldCollision::SetExtents( const Vector& HalfExtents )
{
	// If this entity is a nav blocker, remove its old bounds before updating the extents.
	ConditionalSetNavBlocking( false );

	m_HalfExtents = HalfExtents;

	UpdateBounds();
	UpdateTouching();

	// If this entity is a nav blocker, adds its new bounds after updating the extents.
	ConditionalSetNavBlocking( true );
}

void WBCompEldCollision::GatherTouching( Array<WBEntityRef>& OutTouching ) const
{
	WBEntity* const pThisEntity = GetEntity();

	if( pThisEntity->IsDestroyed() )
	{
		// Special case, untouch everything.
		return;
	}

	if( !m_CanTouch )
	{
		return;
	}

	WBCompEldTransform* const pThisTransform = pThisEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pThisTransform );

	const AABB ThisBox = AABB::CreateFromCenterAndExtents( pThisTransform->GetLocation(), m_HalfExtents );

	const Array<WBCompEldCollision*>&	CollisionComponents	= GetTouchingArray();
	const uint							NumEntities			= CollisionComponents.Size();
	for( uint CollisionEntityIndex = 0; CollisionEntityIndex < NumEntities; ++CollisionEntityIndex )
	{
		WBCompEldCollision* const pCollision = CollisionComponents[ CollisionEntityIndex ];

		ASSERT( pCollision->m_CanTouch );

		WBEntity* const pEntity = pCollision->GetEntity();

		if( pEntity == pThisEntity )
		{
			continue;
		}

		if( pEntity->IsDestroyed() )
		{
			continue;
		}

		const AABB& Box = pCollision->GetBounds();
		if( Box.Intersects( ThisBox ) )
		{
			OutTouching.Insert( pEntity );
		}
	}
}

// NOTE: This does not capture touches that occur during a sweep; as such, it would
// be possible for an object traveling very fast to tunnel through another object
// and never get a touch notification.
void WBCompEldCollision::UpdateTouching()
{
	PROFILE_FUNCTION;

	Array<WBEntityRef> OldTouching = m_Touching;

	Array<WBEntityRef> CurrentTouching;
	GatherTouching( CurrentTouching );

	// Update m_Touching ASAP (before invoking below events)
	// so other things that check the current touching array
	// will get up to date results. (Fixes a bug with trap
	// bolts not damaging things when they spawn touching.)
	m_Touching = CurrentTouching;
	
	// Untouch anything that was touched and now isn't
	FOR_EACH_ARRAY( TouchingIter, OldTouching, WBEntityRef )
	{
		const WBEntityRef& Touching = TouchingIter.GetValue();

		if( CurrentTouching.Search( Touching ).IsNull() )
		{
			SendUntouchEvent( Touching );
		}
	}

	// Touch anything that was not touched and now is
	FOR_EACH_ARRAY( TouchingIter, CurrentTouching, WBEntityRef )
	{
		const WBEntityRef& Touching = TouchingIter.GetValue();

		if( OldTouching.Search( Touching ).IsNull() )
		{
			SendTouchEvent( Touching );
		}
	}
}

void WBCompEldCollision::SendTouchEvent( const WBEntityRef& TouchingEntity )
{
	XTRACE_FUNCTION;

	WBEntity* const pThisEntity = GetEntity();
	WBEntity* const pTouchingEntity = TouchingEntity.Get();
	DEVASSERT( pTouchingEntity );
	WBCompEldCollision* const pTouchingCollision = GET_WBCOMP( pTouchingEntity, EldCollision );
	DEVASSERT( pTouchingCollision );

	pTouchingCollision->AddTouching( pThisEntity );

	{
		WB_MAKE_EVENT( OnTouched, pThisEntity );
		WB_SET_AUTO( OnTouched, Entity, Touched, pTouchingEntity );
		WB_DISPATCH_EVENT( GetEventManager(), OnTouched, pThisEntity );
	}

	{
		WB_MAKE_EVENT( OnTouched, pTouchingEntity );
		WB_SET_AUTO( OnTouched, Entity, Touched, pThisEntity );
		WB_DISPATCH_EVENT( GetEventManager(), OnTouched, pTouchingEntity );
	}
}

void WBCompEldCollision::SendUntouchEvent( const WBEntityRef& TouchingEntity )
{
	XTRACE_FUNCTION;

	WBEntity* const pThisEntity = GetEntity();
	WBEntity* const pTouchingEntity = TouchingEntity.Get();
	WBCompEldCollision* const pTouchingCollision = SAFE_GET_WBCOMP( pTouchingEntity, EldCollision );

	if( pTouchingCollision )
	{
		pTouchingCollision->RemoveTouching( pThisEntity );
	}

	{
		WB_MAKE_EVENT( OnUntouched, pThisEntity );
		WB_SET_AUTO( OnUntouched, Entity, Untouched, pTouchingEntity );
		WB_DISPATCH_EVENT( GetEventManager(), OnUntouched, pThisEntity );
	}

	if( pTouchingEntity )
	{
		WB_MAKE_EVENT( OnUntouched, pTouchingEntity );
		WB_SET_AUTO( OnUntouched, Entity, Untouched, pThisEntity );
		WB_DISPATCH_EVENT( GetEventManager(), OnUntouched, pTouchingEntity );
	}
}

void WBCompEldCollision::GetTouchingEntities( Array<WBEntity*>& OutTouchingEntities ) const
{
	FOR_EACH_ARRAY( TouchingIter, m_Touching, WBEntityRef )
	{
		const WBEntityRef& Touching = TouchingIter.GetValue();
		WBEntity* const pTouchingEntity = Touching.Get();
		if( pTouchingEntity )
		{
			OutTouchingEntities.Insert( pTouchingEntity );
		}
	}
}

void WBCompEldCollision::AddToCollisionMap()
{
	// NOTE: For now, I'm just sorting by blocking type.
	// I could sort by complete flag if I need.
#define CONDITIONAL_ADD_TO_COLLISION_MAP( flag ) if( MatchesAllCollisionFlags( flag ) ) { AddToCollisionArray( flag ); }

	CONDITIONAL_ADD_TO_COLLISION_MAP( EECF_BlocksWorld );
	CONDITIONAL_ADD_TO_COLLISION_MAP( EECF_BlocksEntities );
	CONDITIONAL_ADD_TO_COLLISION_MAP( EECF_BlocksOcclusion );
	CONDITIONAL_ADD_TO_COLLISION_MAP( EECF_BlocksTrace );

#undef CONDITIONAL_ADD_TO_COLLISION_MAP
}

void WBCompEldCollision::AddToCollisionArray( const uint Flags )
{
	TCollisionArray& CollisionArray = sm_CollisionMap[ Flags ];
	ASSERT( !CollisionArray.Find( this, NULL ) );
	CollisionArray.PushBack( this );
}

void WBCompEldCollision::AddToTouchingArray()
{
	if( m_CanTouch )
	{
		ASSERT( !sm_TouchingArray.Find( this, NULL ) );
		sm_TouchingArray.PushBack( this );
	}
}

void WBCompEldCollision::RemoveFromCollisionMap()
{
#define CONDITIONAL_REMOVE_FROM_COLLISION_MAP( flag ) if( MatchesAllCollisionFlags( flag ) ) { RemoveFromCollisionArray( flag ); }

	CONDITIONAL_REMOVE_FROM_COLLISION_MAP( EECF_BlocksWorld );
	CONDITIONAL_REMOVE_FROM_COLLISION_MAP( EECF_BlocksEntities );
	CONDITIONAL_REMOVE_FROM_COLLISION_MAP( EECF_BlocksOcclusion );
	CONDITIONAL_REMOVE_FROM_COLLISION_MAP( EECF_BlocksTrace );

#undef CONDITIONAL_REMOVE_FROM_COLLISION_MAP
}

void WBCompEldCollision::RemoveFromCollisionArray( const uint Flags )
{
	ASSERT( sm_CollisionMap.Search( Flags ).IsValid() );
	TCollisionArray& CollisionArray = sm_CollisionMap[ Flags ];
	ASSERT( CollisionArray.Find( this, NULL ) );
	CollisionArray.FastRemoveItem( this );

	// Clean up after array so we don't need to manage static memory.
	if( CollisionArray.Empty() )
	{
		sm_CollisionMap.Remove( Flags );
	}
}

void WBCompEldCollision::RemoveFromTouchingArray()
{
	if( m_CanTouch )
	{
		ASSERT( sm_TouchingArray.Find( this, NULL ) );
		sm_TouchingArray.FastRemoveItem( this );
	}
}

/*static*/ const WBCompEldCollision::TCollisionArray* WBCompEldCollision::GetCollisionArray( const uint Flags )
{
	TCollisionMap::Iterator CollisionMapIter = sm_CollisionMap.Search( Flags );
	if( CollisionMapIter.IsValid() )
	{
		const TCollisionArray& CollisionArray = CollisionMapIter.GetValue();
		return &CollisionArray;
	}
	else
	{
		return NULL;
	}
}

#define VERSION_EMPTY		0
#define VERSION_TOUCHING	1
#define VERSION_HALFEXTENTS	2
#define VERSION_CANTOUCH	3
#define VERSION_FLAGS		4
#define VERSION_FRICTION	5
#define VERSION_CURRENT		5

uint WBCompEldCollision::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;											// Version
	Size += sizeof( uint32 );							// m_Touching.Size()
	Size += sizeof( WBEntityRef ) * m_Touching.Size();	// m_Touching
	Size += sizeof( Vector );							// m_HalfExtents
	Size += 1;											// m_CanTouch
	Size += 4;											// m_CollisionFlags
	Size += 4;											// m_FrictionTargetTime
	Size += 4;											// m_FrictionTargetPercentVelocity

	return Size;
}

void WBCompEldCollision::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteUInt32( m_Touching.Size() );
	FOR_EACH_ARRAY( TouchingIter, m_Touching, WBEntityRef )
	{
		const WBEntityRef& Touching = TouchingIter.GetValue();
		Stream.Write( sizeof( WBEntityRef ), &Touching );
	}

	Stream.Write( sizeof( Vector ), &m_HalfExtents );

	Stream.WriteBool( m_CanTouch );

	Stream.WriteUInt32( m_CollisionFlags );

	Stream.WriteFloat( m_FrictionTargetTime );
	Stream.WriteFloat( m_FrictionTargetPercentVelocity );
}

void WBCompEldCollision::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_TOUCHING )
	{
		ASSERT( m_Touching.Empty() );
		const uint NumTouching = Stream.ReadUInt32();
		for( uint TouchingIndex = 0; TouchingIndex < NumTouching; ++TouchingIndex )
		{
			WBEntityRef Touching;
			Stream.Read( sizeof( WBEntityRef ), &Touching );
			m_Touching.Insert( Touching );
		}
	}

	if( Version >= VERSION_HALFEXTENTS )
	{
		Stream.Read( sizeof( Vector ), &m_HalfExtents );
	}

	if( Version >= VERSION_CANTOUCH )
	{
		RemoveFromTouchingArray();

		m_CanTouch = Stream.ReadBool();

		AddToTouchingArray();
	}

	if( Version >= VERSION_FLAGS )
	{
		SetCollisionFlags( Stream.ReadUInt32() );
	}

	if( Version >= VERSION_FRICTION )
	{
		m_FrictionTargetTime			= Stream.ReadFloat();
		m_FrictionTargetPercentVelocity	= Stream.ReadFloat();
	}
}