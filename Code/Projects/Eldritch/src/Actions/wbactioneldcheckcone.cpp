#include "core.h"
#include "wbactioneldcheckcone.h"
#include "configmanager.h"
#include "Components/wbcompeldtransform.h"
#include "Components/wbcompeldcollision.h"
#include "Components/wbcompeldcamera.h"
#include "Components/wbcompeldheadtracker.h"
#include "Components/wbcompowner.h"
#include "wbactionstack.h"
#include "wbeventmanager.h"
#include "mathcore.h"
#include "wbcomponentarrays.h"
#include "eldritchframework.h"
#include "eldritchworld.h"
#include "segment.h"
#include "collisioninfo.h"

WBActionEldCheckCone::WBActionEldCheckCone()
:	m_ConeCosTheta( 0.0f )
,	m_ConeLengthSq( 0.0f )
,	m_CheckTag()
{
}

WBActionEldCheckCone::~WBActionEldCheckCone()
{
}

/*virtual*/ void WBActionEldCheckCone::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( ConeAngle );
	m_ConeCosTheta = Cos( DEGREES_TO_RADIANS( ConfigManager::GetFloat( sConeAngle, 0.0f, sDefinitionName ) ) );

	STATICHASH( ConeLength );
	m_ConeLengthSq = Square( ConfigManager::GetFloat( sConeLength, 0.0f, sDefinitionName ) );

	STATICHASH( CheckTag );
	m_CheckTag = ConfigManager::GetHash( sCheckTag, HashedString::NullString, sDefinitionName );
}

// Borrowed (with simplifications) from WBActionEldSpawnEntity::GetSpawnTransform. Maybe unify?
void WBActionEldCheckCone::GetConeTransform( Vector& OutLocation, Angles& OutOrientation ) const
{
	WBEntity* const				pEntity			= GetOwner();
	DEVASSERT( pEntity );
	WBCompEldTransform* const	pTransform		= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );
	WBCompEldCamera* const		pCamera			= GET_WBCOMP( pEntity, EldCamera );
	WBCompEldHeadTracker* const	pHeadTracker	= GET_WBCOMP( pEntity, EldHeadTracker );

	// Get location
	{
		if( pHeadTracker )
		{
			OutLocation			= pHeadTracker->GetEyesLocation();
		}
		else
		{
			OutLocation			= pTransform->GetLocation();
			if( pCamera )
			{
				OutLocation		+= pCamera->GetViewTranslationOffset( WBCompEldCamera::EVM_All );
			}
		}
	}

	// Get orientation
	{
		if( pHeadTracker )
		{
			OutOrientation		= pHeadTracker->GetLookDirection().ToAngles();
		}
		else
		{
			OutOrientation		= pTransform->GetOrientation();
			if( pCamera )
			{
				OutOrientation	+= pCamera->GetViewOrientationOffset( WBCompEldCamera::EVM_All );
			}
		}
	}
}

/*virtual*/ void WBActionEldCheckCone::Execute()
{
	WBAction::Execute();

	const Array<WBCompEldCollision*>* const pCollidables = WBComponentArrays::GetComponents<WBCompEldCollision>();
	if( !pCollidables )
	{
		return;
	}

	STATIC_HASHED_STRING( EventOwner );
	WBEntity* const				pEntity			= GetEntity();
	WBEntity* const				pOwnerEntity	= GetOwner();
	EldritchWorld* const		pWorld			= EldritchFramework::GetInstance()->GetWorld();
	WBEventManager* const		pEventManager	= WBWorld::GetInstance()->GetEventManager();

	Vector						ConeSource;
	Angles						ConeOrientation;
	GetConeTransform( ConeSource, ConeOrientation );
	const Vector				ConeDirection	= ConeOrientation.ToVector();

	const uint NumCollidables = pCollidables->Size();
	for( uint CollidableIndex = 0; CollidableIndex < NumCollidables; ++CollidableIndex )
	{
		WBCompEldCollision* const	pCollision				= ( *pCollidables )[ CollidableIndex ];
		WBEntity* const				pCollidableEntity		= pCollision->GetEntity();
		if( pCollidableEntity == pOwnerEntity )
		{
			continue;
		}

		// Cone check should only hit things that a line check would hit.
		// But since I don't actually do a trace (except to find occlusion),
		// early out of this collidable doesn't block traces.
		if( !pCollision->MatchesAllCollisionFlags( EECF_BlocksTrace ) )
		{
			continue;
		}

		// Check distance to entity's nearest point.
		const AABB					CollidableBounds		= pCollision->GetBounds();
		const Vector				CollidableLocation		= CollidableBounds.GetClosestPoint( ConeSource );
		const Vector				CollidableOffset		= ( CollidableLocation - ConeSource );
		const float					DistanceSq				= CollidableOffset.LengthSquared();
		if( DistanceSq > m_ConeLengthSq )
		{
			continue;
		}

		// Check angle to entity's nearest point. (Skip check if distance to nearest point is zero.)
		// NOTE: This check isn't really sufficient; if ConeDirection is tangential to the surface,
		// the nearest point will be perpendicular to ConeDirection even though some of the surface
		// may fall inside the cone. For now, I'm working around this in data.
		const Vector				CollidableDirection		= CollidableOffset.GetFastNormalized();
		const float					CosTheta				= CollidableDirection.Dot( ConeDirection );
		if( DistanceSq > 0.0f && CosTheta < m_ConeCosTheta )
		{
			continue;
		}

		// Check collision to entity's nearest point.
		const Segment				TraceSegment			= Segment( ConeSource, CollidableLocation );
		CollisionInfo Info;
		Info.m_CollideWorld		= true;
		Info.m_CollideEntities	= true;
		Info.m_CollidingEntity	= pOwnerEntity;
		Info.m_UserFlags		= EECF_Trace;
		if( pWorld->Trace( TraceSegment, Info ) && Info.m_HitEntity != pCollidableEntity )
		{
			continue;
		}

		// All checks passed, notify this entity that the cone check hit that entity.
		WB_MAKE_EVENT( OnConeCheck, pEntity );
		WB_SET_AUTO( OnConeCheck, Entity, Checked, pCollidableEntity );
		WB_SET_AUTO( OnConeCheck, Hash, CheckTag, m_CheckTag );
		WB_SET_AUTO( OnConeCheck, Vector, HitLocation, CollidableLocation );
		WB_DISPATCH_EVENT( pEventManager, OnConeCheck, pEntity );
	}
}