#include "core.h"
#include "wbactioneldcheckline.h"
#include "configmanager.h"
#include "Components/wbcompeldtransform.h"
#include "Components/wbcompeldcamera.h"
#include "Components/wbcompeldheadtracker.h"
#include "Components/wbcompowner.h"
#include "wbactionstack.h"
#include "wbeventmanager.h"
#include "eldritchframework.h"
#include "eldritchworld.h"
#include "segment.h"
#include "ray.h"
#include "collisioninfo.h"

WBActionEldCheckLine::WBActionEldCheckLine()
:	m_LineLength( 0.0f )
,	m_CheckTag()
{
}

WBActionEldCheckLine::~WBActionEldCheckLine()
{
}

/*virtual*/ void WBActionEldCheckLine::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( LineLength );
	m_LineLength = ConfigManager::GetFloat( sLineLength, 0.0f, sDefinitionName );

	STATICHASH( CheckTag );
	m_CheckTag = ConfigManager::GetHash( sCheckTag, HashedString::NullString, sDefinitionName );
}

// Copied from WBActionEldSpawnEntity::GetSpawnTransform. For reals, unify this stuff somewhere.
void WBActionEldCheckLine::GetLineTransform( Vector& OutLocation, Angles& OutOrientation ) const
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

/*virtual*/ void WBActionEldCheckLine::Execute()
{
	WBAction::Execute();

	STATIC_HASHED_STRING( EventOwner );
	WBEntity* const				pEntity			= GetEntity();
	WBEntity* const				pOwnerEntity	= GetOwner();
	EldritchWorld* const		pWorld			= EldritchFramework::GetInstance()->GetWorld();
	WBEventManager* const		pEventManager	= WBWorld::GetInstance()->GetEventManager();

	Vector						LineStart;
	Angles						LineOrientation;
	GetLineTransform( LineStart, LineOrientation );
	const Vector				LineDirection	= LineOrientation.ToVector();

	CollisionInfo Info;
	Info.m_CollideWorld		= true;
	Info.m_CollideEntities	= true;
	Info.m_CollidingEntity	= pOwnerEntity;
	Info.m_UserFlags		= EECF_Trace;

	if( m_LineLength > 0.0f )
	{
		const Vector			LineEnd			= LineStart + LineDirection * m_LineLength;
		const Segment			TraceSegment	= Segment( LineStart, LineEnd );
		if( !pWorld->Trace( TraceSegment, Info ) )
		{
			return;
		}
	}
	else
	{
		const Ray				TraceRay		= Ray( LineStart, LineDirection );
		if( !pWorld->Trace( TraceRay, Info ) )
		{
			return;
		}
	}

	const Vector	HitLocation		= Info.m_Intersection;
	const Vector	HitNormal		= Info.m_Plane.m_Normal;

	// Notify this entity that the line check hit that entity.
	if( Info.m_HitEntity )
	{
		WB_MAKE_EVENT( OnLineCheck, pEntity );
		WB_SET_AUTO( OnLineCheck, Hash, CheckTag, m_CheckTag );
		WB_SET_AUTO( OnLineCheck, Entity, Checked, static_cast<WBEntity*>( Info.m_HitEntity ) );
		WB_SET_AUTO( OnLineCheck, Vector, LineDirection, LineDirection );
		WB_SET_AUTO( OnLineCheck, Vector, HitLocation, HitLocation );
		WB_SET_AUTO( OnLineCheck, Vector, HitNormal, HitNormal );
		WB_DISPATCH_EVENT( pEventManager, OnLineCheck, pEntity );
	}
	else
	{
		const Vector	HalfHitNormal	= 0.5f * Info.m_Plane.m_Normal;
		const Vector	HitVoxel		= pWorld->GetVoxelCenter( HitLocation - HalfHitNormal );

		WB_MAKE_EVENT( OnLineCheckMissed, pEntity );
		WB_SET_AUTO( OnLineCheckMissed, Hash, CheckTag, m_CheckTag );
		WB_SET_AUTO( OnLineCheckMissed, Vector, LineDirection, LineDirection );
		WB_SET_AUTO( OnLineCheckMissed, Vector, HitLocation, HitLocation );
		WB_SET_AUTO( OnLineCheckMissed, Vector, HitNormal, HitNormal );
		WB_SET_AUTO( OnLineCheckMissed, Vector, HitVoxel, HitVoxel );
		WB_DISPATCH_EVENT( pEventManager, OnLineCheckMissed, pEntity );
	}
}