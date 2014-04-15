#include "core.h"
#include "wbcompeldcamera.h"
#include "wbcompeldtransform.h"
#include "configmanager.h"
#include "wbentity.h"
#include "eldritchframework.h"
#include "collisioninfo.h"
#include "eldritchworld.h"
#include "mathcore.h"
#include "matrix.h"
#include "segment.h"
#include "wbeventmanager.h"

WBCompEldCamera::WBCompEldCamera()
:	m_ViewOffsetZ( 0.0f )
,	m_LastViewOffsetZ( 0.0f )
,	m_ViewAngleOffsetRoll( 0.0f )
,	m_LeanRoll( 0.0f )
,	m_LeanPosition( 0.0f )
,	m_LeanVelocity( 0.0f )
,	m_LeanRollMax( 0.0f )
,	m_LeanRadius( 0.0f )
,	m_LeanExtent( 0.0f )
{
}

WBCompEldCamera::~WBCompEldCamera()
{
}

/*virtual*/ void WBCompEldCamera::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( ViewOffsetZ );
	m_ViewOffsetZ = ConfigManager::GetInheritedFloat( sViewOffsetZ, 0.0f, sDefinitionName );
	m_LastViewOffsetZ = m_ViewOffsetZ;

	STATICHASH( LeanVelocity );
	m_LeanVelocity = DEGREES_TO_RADIANS( ConfigManager::GetInheritedFloat( sLeanVelocity, 0.0f, sDefinitionName ) );

	STATICHASH( LeanRollMax );
	m_LeanRollMax = DEGREES_TO_RADIANS( ConfigManager::GetInheritedFloat( sLeanRollMax, 0.0f, sDefinitionName ) );

	STATICHASH( LeanRadius );
	m_LeanRadius = ConfigManager::GetInheritedFloat( sLeanRadius, 0.0f, sDefinitionName );

	STATICHASH( LeanExtent );
	m_LeanExtent = ConfigManager::GetInheritedFloat( sLeanExtent, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldCamera::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( SetViewOffsetZ );
	STATIC_HASHED_STRING( ResetViewOffsetZ );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sSetViewOffsetZ )
	{
		STATIC_HASHED_STRING( ViewOffsetZ );
		const float ViewOffsetZ = Event.GetFloat( sViewOffsetZ );
		SetViewOffsetZ( ViewOffsetZ );
	}
	else if( EventName == sResetViewOffsetZ )
	{
		// Hack for reviving player into correct view state
		SetViewOffsetZ( m_LastViewOffsetZ );
	}
}

/*virtual*/ void WBCompEldCamera::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	Unused( DeltaTime );

	const float DesiredRoll = GetDesiredLean( m_LeanPosition );
	UpdateLean( DesiredRoll, DeltaTime );

	WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const Vector ViewTranslation = pTransform->GetLocation() + GetViewTranslationOffset( EVM_All );
	const Angles ViewOrientation = pTransform->GetOrientation() + GetViewOrientationOffset( EVM_All );
	EldritchFramework::GetInstance()->SetMainViewTransform( ViewTranslation, ViewOrientation );

	WB_MAKE_EVENT( OnCameraTicked, GetEntity() );
	WB_DISPATCH_EVENT( GetEventManager(), OnCameraTicked, NULL );
}

void WBCompEldCamera::SetViewOffsetZ( const float ViewOffsetZ )
{
	m_LastViewOffsetZ	= m_ViewOffsetZ;
	m_ViewOffsetZ		= ViewOffsetZ;
}

Vector WBCompEldCamera::GetViewTranslationOffset( const WBCompEldCamera::EViewModifiers Modifiers ) const
{
	Vector Offset;

	if( Modifiers & EVM_OffsetZ )
	{
		Offset.z += m_ViewOffsetZ;
	}

	if( Modifiers & EVM_Lean )
	{
		Offset += GetLeanOffset( m_LeanRoll );
	}

	return Offset;
}

Angles WBCompEldCamera::GetViewOrientationOffset( const WBCompEldCamera::EViewModifiers Modifiers ) const
{
	Angles Offset;

	if( Modifiers & EVM_Roll )
	{
		Offset.Roll += m_ViewAngleOffsetRoll;
	}

	if( Modifiers & EVM_Lean )
	{
		Offset.Roll += m_LeanRoll;
	}

	return Offset;
}

Vector WBCompEldCamera::GetLeanOffset( const float LeanRoll ) const
{
	if( LeanRoll == 0.0f )
	{
		// Small optimization.
		return Vector();
	}
	else
	{
		WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
		DEVASSERT( pTransform );

		const Angles	LeanAngle	= Angles( 0.0f, LeanRoll, pTransform->GetOrientation().Yaw );
		Vector			LeanVector	= Vector( 0.0f, 0.0f, m_LeanRadius );

		LeanVector		*= LeanAngle.ToMatrix();
		LeanVector.z	-= m_LeanRadius;

		return LeanVector;
	}
}

float WBCompEldCamera::GetDesiredLean( const float LeanPosition ) const
{
	WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const float DesiredRoll = m_LeanRollMax * LeanPosition;

	const Vector	ViewBase	= pTransform->GetLocation() + GetViewTranslationOffset( EVM_OffsetZ );
	const Vector	LeanOffset	= GetLeanOffset( DesiredRoll );
	const Segment	LeanSegment	= Segment( ViewBase, ViewBase + LeanOffset );
	const Vector	LeanExtents = Vector( m_LeanExtent, m_LeanExtent, m_LeanExtent );

	CollisionInfo Info;
	Info.m_CollideWorld		= true;
	Info.m_CollideEntities	= true;
	Info.m_CollidingEntity	= GetEntity();
	Info.m_UserFlags		= EECF_EntityCollision;

	if( GetWorld()->Sweep( LeanSegment, LeanExtents, Info ) )
	{
		return m_LeanRollMax * Info.m_HitT * Sign( LeanPosition );
	}
	else
	{
		return DesiredRoll;
	}
}

void WBCompEldCamera::UpdateLean( const float TargetRoll, const float DeltaTime )
{
	const float LeanDelta = m_LeanVelocity * DeltaTime;

	if( m_LeanRoll < TargetRoll )
	{
		m_LeanRoll = Min( m_LeanRoll + LeanDelta, TargetRoll );
	}
	else if( m_LeanRoll > TargetRoll )
	{
		m_LeanRoll = Max( m_LeanRoll - LeanDelta, TargetRoll );
	}
}