#include "core.h"
#include "wbcompeldsensorvision.h"
#include "configmanager.h"
#include "mathcore.h"
#include "eldritchframework.h"
#include "wbcompeldvisible.h"
#include "wbcomponentarrays.h"
#include "wbcompeldtransform.h"
#include "wbcompeldheadtracker.h"
#include "Components/wbcomprodinknowledge.h"
#include "collisioninfo.h"
#include "irenderer.h"
#include "eldritchworld.h"

WBCompEldSensorVision::WBCompEldSensorVision()
:	m_EyeOffsetZ( 0.0f )
,	m_RadiusSq( 0.0f )
,	m_ConeCos( 0.0f )
,	m_ConeInvZScale( 0.0f )
,	m_CertaintyFalloffRadius( 0.0f )
,	m_DistanceCertaintyFactor( 0.0f )
,	m_CertaintyVelocity( 0.0f )
,	m_CertaintyDecay( 0.0f )
{
}

WBCompEldSensorVision::~WBCompEldSensorVision()
{
}

/*virtual*/ void WBCompEldSensorVision::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( EyeOffsetZ );
	m_EyeOffsetZ = ConfigManager::GetInheritedFloat( sEyeOffsetZ, 0.0f, sDefinitionName );

	STATICHASH( Radius );
	m_RadiusSq = Square( ConfigManager::GetInheritedFloat( sRadius, 0.0f, sDefinitionName ) );

	STATICHASH( ConeAngle );
	m_ConeCos = Cos( DEGREES_TO_RADIANS( ConfigManager::GetInheritedFloat( sConeAngle, 0.0f, sDefinitionName ) ) );

	STATICHASH( ConeScaleZ );
	const float ConeScaleZ = ConfigManager::GetInheritedFloat( sConeScaleZ, 0.0f, sDefinitionName );
	m_ConeInvZScale = ( ConeScaleZ > 0.0f ) ? ( 1.0f / ConeScaleZ ) : ConeScaleZ;

	STATICHASH( CertaintyFalloffRadius );
	m_CertaintyFalloffRadius = ConfigManager::GetInheritedFloat( sCertaintyFalloffRadius, 0.0f, sDefinitionName );

	STATICHASH( DistanceCertaintyFactor );
	m_DistanceCertaintyFactor = ConfigManager::GetInheritedFloat( sDistanceCertaintyFactor, 0.0f, sDefinitionName );

	STATICHASH( CertaintyVelocity );
	m_CertaintyVelocity = ConfigManager::GetInheritedFloat( sCertaintyVelocity, 0.0f, sDefinitionName );

	STATICHASH( CertaintyDecay );
	m_CertaintyDecay = ConfigManager::GetInheritedFloat( sCertaintyDecay, 0.0f, sDefinitionName );
}

// Keep in sync with PollTick! I'd unify it, but I want that to be optimized.
bool WBCompEldSensorVision::IsVisible( WBEntity* const pVisibleEntity ) const
{
	ASSERT( pVisibleEntity );

	WBCompEldVisible*			pVisibleComponent	= GET_WBCOMP( pVisibleEntity, EldVisible );
	if( !pVisibleComponent )
	{
		return false;
	}

	EldritchWorld* const		pWorld			= GetWorld();
	ASSERT( pWorld );

	WBEntity* const				pEntity			= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform		= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldHeadTracker* const	pHeadTracker	= GET_WBCOMP( pEntity, EldHeadTracker );

	const Vector				Location		= pTransform->GetLocation();
	const Vector				EyeLocation		= Location + Vector( 0.0f, 0.0f, m_EyeOffsetZ );
	const Vector				EyeDirection	= pHeadTracker ? pHeadTracker->GetLookDirection() : pTransform->GetOrientation().ToVector();

	static const Vector			Up				= Vector( 0.0f, 0.0f, 1.0f );
	const Vector				EyeRight		= EyeDirection.Cross( Up ).GetNormalized();
	const Vector				EyeUp			= EyeRight.Cross( EyeDirection );

	if( !pVisibleComponent->IsVisible() )
	{
		return false;
	}

	// Distance check
	const Vector	VisibleLocation	= pVisibleComponent->GetVisibleLocation();
	const Vector	VisibleOffset	= VisibleLocation - EyeLocation;
	const float		VisibleDistSq	= VisibleOffset.LengthSquared();

	if( VisibleDistSq > m_RadiusSq )
	{
		// Entity is beyond vision distance.
		return false;
	}

	// Cone check, with scaling on Z to flatten the cone
	const Vector	OffsetUpPart		= VisibleOffset.ProjectionOnto( EyeUp );
	const Vector	OffsetOtherPart		= VisibleOffset - OffsetUpPart;
	const Vector	EffectiveOffset		= OffsetOtherPart + OffsetUpPart * m_ConeInvZScale;
	const Vector	EffectiveDirection	= EffectiveOffset.GetFastNormalized();
	const float		EffectiveCos		= EyeDirection.Dot( EffectiveDirection );
	if( EffectiveCos < m_ConeCos )
	{
		// Entity is outside view cone.
		return false;
	}

	// World line check
	CollisionInfo Info;
	Info.m_CollideWorld			= true;
	Info.m_CollideEntities		= true;
	Info.m_UserFlags			= EECF_Occlusion;
	Info.m_StopAtAnyCollision	= true;
	if( pWorld->LineCheck( EyeLocation, VisibleLocation, Info ) )
	{
		// Entity is occluded.
		return false;
	}

	return true;
}

/*virtual*/ void WBCompEldSensorVision::PollTick( const float DeltaTime ) const
{
	const Array<WBCompEldVisible*>* pVisibleComponents = WBComponentArrays::GetComponents<WBCompEldVisible>();
	if( !pVisibleComponents )
	{
		return;
	}

	EldritchWorld* const		pWorld			= GetWorld();
	ASSERT( pWorld );

	WBEntity* const				pEntity			= GetEntity();
	DEVASSERT( pEntity );

	WBCompRodinKnowledge* const	pKnowledge		= GET_WBCOMP( pEntity, RodinKnowledge );
	ASSERT( pKnowledge );

	WBCompEldTransform* const	pTransform		= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldHeadTracker* const	pHeadTracker	= GET_WBCOMP( pEntity, EldHeadTracker );

	const Vector				Location		= pTransform->GetLocation();
	const Vector				EyeLocation		= Location + Vector( 0.0f, 0.0f, m_EyeOffsetZ );
	const Vector				EyeDirection	= pHeadTracker ? pHeadTracker->GetLookDirection() : pTransform->GetOrientation().ToVector();

	static const Vector			Up				= Vector( 0.0f, 0.0f, 1.0f );
	const Vector				EyeRight		= EyeDirection.Cross( Up ).GetNormalized();
	const Vector				EyeUp			= EyeRight.Cross( EyeDirection );

	const uint NumVisibles = pVisibleComponents->Size();
	for( uint VisibleIndex = 0; VisibleIndex < NumVisibles; ++VisibleIndex )
	{
		WBCompEldVisible* const	pVisible		= ( *pVisibleComponents )[ VisibleIndex ];
		ASSERT( pVisible );

		WBEntity* const			pVisibleEntity	= pVisible->GetEntity();
		ASSERT( pVisibleEntity );

		WBCompRodinKnowledge::TKnowledge* const pKnowledgeEntry = pKnowledge->GetKnowledge( pVisibleEntity );
		if( pKnowledgeEntry )
		{
			// Decay knowledge if there is any
			STATIC_HASHED_STRING( VisionCertainty );
			float VisionCertainty = pKnowledgeEntry->GetFloat( sVisionCertainty );
			VisionCertainty = Max( 0.0f, VisionCertainty - m_CertaintyDecay * DeltaTime );
			pKnowledgeEntry->SetFloat( sVisionCertainty, VisionCertainty );
		}

		if( !pVisible->IsVisible() )
		{
			continue;
		}

		// Don't check visibility of self
		if( pVisibleEntity == pEntity )
		{
			continue;
		}

		// Distance check
		const Vector	VisibleLocation	= pVisible->GetVisibleLocation();
		const Vector	VisibleOffset	= VisibleLocation - EyeLocation;
		const float		VisibleDistSq	= VisibleOffset.LengthSquared();
		if( VisibleDistSq > m_RadiusSq )
		{
			// Entity is beyond vision distance.
			continue;
		}

		// Cone check, with scaling on Z to flatten the cone
		const Vector	OffsetUpPart		= VisibleOffset.ProjectionOnto( EyeUp );
		const Vector	OffsetOtherPart		= VisibleOffset - OffsetUpPart;
		const Vector	EffectiveOffset		= OffsetOtherPart + OffsetUpPart * m_ConeInvZScale;
		const Vector	EffectiveDirection	= EffectiveOffset.GetFastNormalized();
		const float		EffectiveCos		= EyeDirection.Dot( EffectiveDirection );
		if( EffectiveCos < m_ConeCos )
		{
			// Entity is outside view cone.
			continue;
		}

		// World line check
		CollisionInfo Info;
		Info.m_CollideWorld			= true;
		Info.m_CollideEntities		= true;
		Info.m_UserFlags			= EECF_Occlusion;
		Info.m_StopAtAnyCollision	= true;
		if( pWorld->LineCheck( EyeLocation, VisibleLocation, Info ) )
		{
			// Entity is occluded.
			continue;
		}

		// Entity is visible.
		WBCompEldTransform* const	pVisibleTransform	= pVisibleEntity->GetTransformComponent<WBCompEldTransform>();
		ASSERT( pVisibleTransform );

		const Vector				SeenLocation			= pVisibleTransform->GetLocation();
		const float					SeenDistance			= ( SeenLocation - Location ).Length();
		const float					DistanceCertainty		= Attenuate( SeenDistance, m_CertaintyFalloffRadius );
		const float					DistanceCertaintyFactor	= Lerp( 1.0f - m_DistanceCertaintyFactor, 1.0f, DistanceCertainty );

		// TODO: Multiply other factors in here (luminance, periphery, velocity, crouching, etc.)
		const float					Certainty				= DistanceCertaintyFactor;

		WBCompRodinKnowledge::TKnowledge& Knowledge = pKnowledge->UpdateEntity( pVisibleEntity );

		STATIC_HASHED_STRING( VisionCertainty );
		float VisionCertainty = Knowledge.GetFloat( sVisionCertainty );
		// This is a bit kludgy. Accelerate toward certainty faster if target certainty is higher.
		VisionCertainty = Min( Certainty, VisionCertainty + m_CertaintyVelocity * Certainty * DeltaTime );
		Knowledge.SetFloat( sVisionCertainty, VisionCertainty );

		STATIC_HASHED_STRING( LastKnownLocation );
		Knowledge.SetVector( sLastKnownLocation, VisibleLocation );
		ASSERT( !VisibleLocation.IsZero() );

		STATIC_HASHED_STRING( LastSeenLocation );
		Knowledge.SetVector( sLastSeenLocation, VisibleLocation );

		STATIC_HASHED_STRING( LastSeenTime );
		Knowledge.SetFloat( sLastSeenTime, GetTime() );

		STATIC_HASHED_STRING( KnowledgeType );
		STATIC_HASHED_STRING( Target );
		Knowledge.SetHash( sKnowledgeType, sTarget );
	}
}

#if BUILD_DEV
/*virtual*/ void WBCompEldSensorVision::DebugRender() const
{
	WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const Vector EyeLocation = pTransform->GetLocation() + Vector( 0.0f, 0.0f, m_EyeOffsetZ );

	GetFramework()->GetRenderer()->DEBUGDrawCross( EyeLocation, 0.5f, ARGB_TO_COLOR( 255, 255, 0, 0 ) );
}
#endif