#include "core.h"
#include "wbactioneldchecksphere.h"
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

WBActionEldCheckSphere::WBActionEldCheckSphere()
:	m_RadiusSq( 0.0f )
,	m_CheckTag()
{
}

WBActionEldCheckSphere::~WBActionEldCheckSphere()
{
}

/*virtual*/ void WBActionEldCheckSphere::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Radius );
	m_RadiusSq = Square( ConfigManager::GetFloat( sRadius, 0.0f, sDefinitionName ) );

	STATICHASH( CheckTag );
	m_CheckTag = ConfigManager::GetHash( sCheckTag, HashedString::NullString, sDefinitionName );
}

// Borrowed (with simplifications) from WBActionEldSpawnEntity::GetSpawnTransform. Maybe unify?
void WBActionEldCheckSphere::GetSphereTransform( Vector& OutLocation ) const
{
	WBEntity* const				pEntity			= GetEntity();	//GetOwner();	(Unlike cone and line checks, I'm not using this on a carried weapon)
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
}

/*virtual*/ void WBActionEldCheckSphere::Execute()
{
	WBAction::Execute();

	const Array<WBCompEldCollision*>* const pCollidables = WBComponentArrays::GetComponents<WBCompEldCollision>();
	if( !pCollidables )
	{
		return;
	}

	WBEntity* const				pEntity			= GetEntity();
	WBEntity* const				pOwnerEntity	= pEntity;	//GetOwner();	(Unlike cone and line checks, I'm not using this on a carried weapon)
	EldritchWorld* const		pWorld			= EldritchFramework::GetInstance()->GetWorld();
	WBEventManager* const		pEventManager	= WBWorld::GetInstance()->GetEventManager();

	Vector						SphereSource;
	GetSphereTransform( SphereSource );

	const uint NumCollidables = pCollidables->Size();
	for( uint CollidableIndex = 0; CollidableIndex < NumCollidables; ++CollidableIndex )
	{
		WBCompEldCollision* const	pCollision				= ( *pCollidables )[ CollidableIndex ];
		WBEntity* const				pCollidableEntity		= pCollision->GetEntity();
		if( pCollidableEntity == pOwnerEntity )
		{
			continue;
		}

		// Check distance to entity's nearest point.
		const AABB					CollidableBounds		= pCollision->GetBounds();
		const Vector				CollidableLocation		= CollidableBounds.GetClosestPoint( SphereSource );
		const Vector				CollidableOffset		= ( CollidableLocation - SphereSource );
		const float					DistanceSq				= CollidableOffset.LengthSquared();
		if( DistanceSq > m_RadiusSq )
		{
			continue;
		}

		// Check collision to entity's nearest point.
		const Segment				TraceSegment			= Segment( SphereSource, CollidableLocation );
		CollisionInfo Info;
		Info.m_CollideWorld		= true;
		Info.m_CollideEntities	= true;
		Info.m_CollidingEntity	= pOwnerEntity;
		Info.m_UserFlags		= EECF_Trace;
		if( pWorld->Trace( TraceSegment, Info ) && Info.m_HitEntity != pCollidableEntity )
		{
			continue;
		}

		// All checks passed, notify this entity that the sphere check hit that entity.
		WB_MAKE_EVENT( OnSphereCheck, pEntity );
		WB_SET_AUTO( OnSphereCheck, Entity, Checked, pCollidableEntity );
		WB_SET_AUTO( OnSphereCheck, Hash, CheckTag, m_CheckTag );
		WB_SET_AUTO( OnSphereCheck, Vector, HitLocation, CollidableLocation );
		WB_DISPATCH_EVENT( pEventManager, OnSphereCheck, pEntity );
	}
}