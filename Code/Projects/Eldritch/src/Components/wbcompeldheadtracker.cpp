#include "core.h"
#include "wbcompeldheadtracker.h"
#include "wbevent.h"
#include "hashedstring.h"
#include "configmanager.h"
#include "wbcompeldtransform.h"
#include "wbcompeldvisible.h"
#include "wbcompeldmesh.h"
#include "Components/wbcomprodinknowledge.h"
#include "matrix.h"
#include "mathcore.h"
#include "bonearray.h"
#include "eldritchmesh.h"
#include "eldritchframework.h"
#include "irenderer.h"

WBCompEldHeadTracker::WBCompEldHeadTracker()
:	m_TrackMode( ETM_None )
,	m_HeadBoneName()
,	m_HeadOffset()
,	m_EyesOffset()
,	m_MaxRotationRadians( 0.0f )
,	m_LookVelocity( 0.0f )
,	m_LookAtTargetEntity()
,	m_LookAtTargetLocation()
,	m_LookRotationOS()
{
}

WBCompEldHeadTracker::~WBCompEldHeadTracker()
{
}

/*virtual*/ void WBCompEldHeadTracker::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( HeadBoneName );
	m_HeadBoneName = ConfigManager::GetInheritedHash( sHeadBoneName, HashedString::NullString, sDefinitionName );

	STATICHASH( HeadOffsetX );
	m_HeadOffset.x = ConfigManager::GetInheritedFloat( sHeadOffsetX, 0.0f, sDefinitionName );

	STATICHASH( HeadOffsetY );
	m_HeadOffset.y = ConfigManager::GetInheritedFloat( sHeadOffsetY, 0.0f, sDefinitionName );

	STATICHASH( HeadOffsetZ );
	m_HeadOffset.z = ConfigManager::GetInheritedFloat( sHeadOffsetZ, 0.0f, sDefinitionName );

	STATICHASH( EyesOffsetX );
	m_EyesOffset.x = ConfigManager::GetInheritedFloat( sEyesOffsetX, 0.0f, sDefinitionName );

	STATICHASH( EyesOffsetY );
	m_EyesOffset.y = ConfigManager::GetInheritedFloat( sEyesOffsetY, 0.0f, sDefinitionName );

	STATICHASH( EyesOffsetZ );
	m_EyesOffset.z = ConfigManager::GetInheritedFloat( sEyesOffsetZ, 0.0f, sDefinitionName );

	STATICHASH( MaxRotation );
	m_MaxRotationRadians = DEGREES_TO_RADIANS( ConfigManager::GetInheritedFloat( sMaxRotation, 0.0f, sDefinitionName ) );

	STATICHASH( LookVelocity );
	m_LookVelocity = DEGREES_TO_RADIANS( ConfigManager::GetInheritedFloat( sLookVelocity, 0.0f, sDefinitionName ) );
}

/*virtual*/ void WBCompEldHeadTracker::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( LookAt );
	STATIC_HASHED_STRING( StopLooking );
	STATIC_HASHED_STRING( OnInitialized );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnInitialized )
	{
		WBCompEldMesh* const pMesh = GET_WBCOMP( GetEntity(), EldMesh );
		ASSERT( pMesh );
		ASSERT( pMesh->GetMesh() );
		pMesh->GetMesh()->AddBoneModifier( this );
	}
	else if( EventName == sLookAt )
	{
		STATIC_HASHED_STRING( LookAtLocation );
		const Vector LookTargetLocation = Event.GetVector( sLookAtLocation );

		STATIC_HASHED_STRING( LookAtEntity );
		WBEntity* const pLookTargetEntity = Event.GetEntity( sLookAtEntity );

		if( pLookTargetEntity )
		{
			LookAtEntity( pLookTargetEntity );
		}
		else
		{
			LookAtLocation( LookTargetLocation );
		}
	}
	else if( EventName == sStopLooking )
	{
		StopLooking();
	}
}

/*virtual*/ void WBCompEldHeadTracker::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	WBEntity* const pEntity = GetEntity();
	DEVASSERT( pEntity );
	WBCompEldTransform* const pTransform = pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	// NOTE: Because we don't have the animation matrices at this point, we can't get the eyes' actual transformed location.
	const Vector EyesLocation = pTransform->GetLocation() + ( m_EyesOffset * pTransform->GetOrientation().ToMatrix() );

	Vector TargetLocation;
	Vector DirectionToTarget;

	if( m_TrackMode == ETM_Entity )
	{
		WBEntity* const pLookAtTarget = m_LookAtTargetEntity.Get();
		if( pLookAtTarget )
		{
			WBCompEldTransform* const pLookAtTransform = pLookAtTarget->GetTransformComponent<WBCompEldTransform>();
			ASSERT( pLookAtTransform );

			WBCompEldVisible* const	pVisible = GET_WBCOMP( pLookAtTarget, EldVisible );

			// Simulational honesty!
			WBCompRodinKnowledge* const pKnowledge = GET_WBCOMP( pEntity, RodinKnowledge );
			if( pKnowledge->IsCurrentlyVisible( pLookAtTarget ) )
			{
				TargetLocation		= pVisible ? pVisible->GetVisibleLocation() : pLookAtTransform->GetLocation();
				DirectionToTarget	= TargetLocation - EyesLocation;
			}
			else
			{
				const bool HasLastKnownLocation = pKnowledge->GetLastKnownLocationFor( pLookAtTarget, TargetLocation );
				if( HasLastKnownLocation )
				{
					// We're good, we've got a last known location
					DirectionToTarget	= TargetLocation - EyesLocation;
				}
				else
				{
					// We don't know where our target is
					StopLooking();
				}
			}
		}
		else
		{
			// Our look-at target has been destroyed
			StopLooking();
		}
	}
	else if( m_TrackMode == ETM_Location )
	{
		TargetLocation		= m_LookAtTargetLocation;
		DirectionToTarget	= TargetLocation - EyesLocation;
	}

	Angles ReverseOwnerRotation = -pTransform->GetOrientation();

	const bool		IsLooking			= m_TrackMode != ETM_None;
	const Matrix	RotationToTarget	= DirectionToTarget.ToAngles().ToMatrix();
	const Matrix	RotationOS			= RotationToTarget * ReverseOwnerRotation.ToMatrix();
	const Quat		ToTarget			= IsLooking ? RotationOS.ToQuaternion() : Quat();
	const Quat		ConstrainedToTarget	= ( ToTarget.GetAngle() <= m_MaxRotationRadians ) ? ToTarget : Quat::CreateRotation( ToTarget.GetAxis(), m_MaxRotationRadians );
	const Quat		Rotation			= ConstrainedToTarget - m_LookRotationOS;
	const float		AmountToTurn		= Min( m_LookVelocity * DeltaTime, Rotation.Length() );	// Prevent overturning

	m_LookRotationOS += Rotation.GetNormalized() * AmountToTurn;
}

/*virtual*/ void WBCompEldHeadTracker::Modify( const BoneArray* pBones, Matrix* pInOutBoneMatrices )
{
	uint HeadBoneIndex = pBones->GetBoneIndex( m_HeadBoneName );
	if( HeadBoneIndex == INVALID_INDEX )
	{
		return;
	}

	Matrix&			HeadBoneMatrix			= pInOutBoneMatrices[ HeadBoneIndex ];

	Matrix			HeadTrackRotation		= m_LookRotationOS.ToMatrix();
	const Vector	TransformedHeadOffset	= m_HeadOffset * HeadBoneMatrix;
	const Vector	HeadTrackedHeadOffset	= TransformedHeadOffset * HeadTrackRotation;
	HeadTrackRotation.SetTranslationElements( TransformedHeadOffset - HeadTrackedHeadOffset );

	HeadBoneMatrix *= HeadTrackRotation;
}

void WBCompEldHeadTracker::LookAtEntity( WBEntity* const pLookAtTarget )
{
	m_TrackMode				= ETM_Entity;
	m_LookAtTargetEntity	= pLookAtTarget;
}

void WBCompEldHeadTracker::LookAtLocation( const Vector& LookAtTarget )
{
	m_TrackMode				= ETM_Location;
	m_LookAtTargetLocation	= LookAtTarget;
	m_LookAtTargetEntity	= NULL;
}

void WBCompEldHeadTracker::StopLooking()
{
	m_TrackMode		= ETM_None;
}

#if BUILD_DEV
/*virtual*/ void WBCompEldHeadTracker::DebugRender() const
{
	WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldMesh* const pMesh = GET_WBCOMP( GetEntity(), EldMesh );
	ASSERT( pMesh );

	const Vector HeadLocation	= pTransform->GetLocation() + pMesh->GetMeshOffset() + ( m_HeadOffset * pTransform->GetOrientation().ToMatrix() );
	const Vector EyeLocation	= pTransform->GetLocation() + ( m_EyesOffset * pTransform->GetOrientation().ToMatrix() );

	GetFramework()->GetRenderer()->DEBUGDrawCross( HeadLocation, 0.25f, ARGB_TO_COLOR( 255, 128, 0, 255 ) );
	GetFramework()->GetRenderer()->DEBUGDrawCross( EyeLocation, 0.25f, ARGB_TO_COLOR( 255, 128, 0, 255 ) );

	const Vector LookDirectionOS = Vector( 0.0f, 1.0f, 0.0f ) * m_LookRotationOS.ToMatrix();
	const Vector LookDirectionWS = LookDirectionOS * pTransform->GetOrientation().ToMatrix();
	GetFramework()->GetRenderer()->DEBUGDrawLine( EyeLocation - LookDirectionWS * 5.0f, EyeLocation + LookDirectionWS * 5.0f, ARGB_TO_COLOR( 255, 255, 255, 255 ) );
	GetFramework()->GetRenderer()->DEBUGDrawLine( HeadLocation - LookDirectionWS * 5.0f, HeadLocation + LookDirectionWS * 5.0f, ARGB_TO_COLOR( 255, 255, 255, 255 ) );
}
#endif

Vector WBCompEldHeadTracker::GetEyesLocation() const
{
	WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const Vector EyesLocation = pTransform->GetLocation() + ( m_EyesOffset * pTransform->GetOrientation().ToMatrix() );

	return EyesLocation;
}

Vector WBCompEldHeadTracker::GetLookDirection() const
{
	WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const Vector LookDirectionOS = Vector( 0.0f, 1.0f, 0.0f ) * m_LookRotationOS.ToMatrix();
	const Vector LookDirectionWS = LookDirectionOS * pTransform->GetOrientation().ToMatrix();

	return LookDirectionWS;
}