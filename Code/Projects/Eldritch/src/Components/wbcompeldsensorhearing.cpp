#include "core.h"
#include "wbcompeldsensorhearing.h"
#include "configmanager.h"
#include "wbeventmanager.h"
#include "Components/wbcomprodinknowledge.h"
#include "wbcompeldtransform.h"
#include "mathcore.h"
#include "collisioninfo.h"
#include "eldritchworld.h"

WBCompEldSensorHearing::WBCompEldSensorHearing()
:	m_Radius( 0.0f )
,	m_CertaintyFalloffRadius( 0.0f )
,	m_DistanceCertaintyFactor( 0.0f )
,	m_OcclusionCertaintyFactor( 0.0f )
{
	STATIC_HASHED_STRING( OnAINoise );
	GetEventManager()->AddObserver( sOnAINoise, this );
}

WBCompEldSensorHearing::~WBCompEldSensorHearing()
{
	WBEventManager* const pEventManager = GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( OnAINoise );
		pEventManager->RemoveObserver( sOnAINoise, this );
	}
}

/*virtual*/ void WBCompEldSensorHearing::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Radius );
	m_Radius = ConfigManager::GetInheritedFloat( sRadius, 0.0f, sDefinitionName );

	STATICHASH( CertaintyFalloffRadius );
	m_CertaintyFalloffRadius = ConfigManager::GetInheritedFloat( sCertaintyFalloffRadius, 0.0f, sDefinitionName );

	STATICHASH( DistanceCertaintyFactor );
	m_DistanceCertaintyFactor = ConfigManager::GetInheritedFloat( sDistanceCertaintyFactor, 0.0f, sDefinitionName );

	STATICHASH( OcclusionCertaintyFactor );
	m_OcclusionCertaintyFactor = ConfigManager::GetInheritedFloat( sOcclusionCertaintyFactor, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldSensorHearing::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnAINoise );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnAINoise )
	{
		STATIC_HASHED_STRING( EventOwner );
		WBEntity* const pEventOwner = Event.GetEntity( sEventOwner );
		ASSERT( pEventOwner );

		if( pEventOwner == GetEntity() )
		{
			// Ignore AI noises from self (even if there's a separate noise entity).
			// This prevents endlessly making self aware of the same target.
			// (Though two AIs could ping-pong awareness... Maybe should decay
			// certainty over time? As a parameter of the AI noise event?)
		}
		else
		{
			STATIC_HASHED_STRING( NoiseEntity );
			WBEntity* const pNoiseEntity = Event.GetEntity( sNoiseEntity );

			WBEntity* const pUseNoiseEntity = pNoiseEntity ? pNoiseEntity : pEventOwner;

			// NOTE: Using a parameter instead of the entity's actual transform.
			STATIC_HASHED_STRING( NoiseLocation );
			const Vector NoiseLocation = Event.GetVector( sNoiseLocation );

			STATIC_HASHED_STRING( NoiseRadius );
			const float NoiseRadius = Event.GetFloat( sNoiseRadius );

			STATIC_HASHED_STRING( NoiseUpdateTime );
			const float NoiseUpdateTime = Event.GetFloat( sNoiseUpdateTime );

			STATIC_HASHED_STRING( ExpireTimeBonus );
			const float ExpireTimeBonus = Event.GetFloat( sExpireTimeBonus );

			HandleNoise( pUseNoiseEntity, NoiseLocation, NoiseRadius, NoiseUpdateTime, ExpireTimeBonus );
		}
	}
}

bool WBCompEldSensorHearing::GetNoiseCertainty( const Vector& NoiseLocation, const float NoiseRadius, float& OutCertainty ) const
{
	EldritchWorld* const		pWorld			= GetWorld();
	ASSERT( pWorld );

	WBEntity* const				pEntity			= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform		= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const Vector				EarsLocation	= pTransform->GetLocation();
	const float					EffectiveRadius	= Max( 0.0f, m_Radius + NoiseRadius );
	const float					RadiusSumSq		= Square( EffectiveRadius );
	const float					DistanceSq		= ( NoiseLocation - EarsLocation ).LengthSquared();

	if( RadiusSumSq < DistanceSq )
	{
		return false;
	}

	CollisionInfo Info;
	Info.m_CollideWorld			= true;
	Info.m_CollideEntities		= true;
	Info.m_UserFlags			= EECF_Occlusion;
	Info.m_StopAtAnyCollision	= true;
	const bool							Occluded					= pWorld->LineCheck( EarsLocation, NoiseLocation, Info );

	const float							HeardDistance				= SqRt( DistanceSq );
	const float							DistanceCertainty			= Attenuate( HeardDistance, m_CertaintyFalloffRadius );
	const float							DistanceCertaintyFactor		= Lerp( 1.0f - m_DistanceCertaintyFactor, 1.0f, DistanceCertainty );

	const float							OcclusionCertainty			= Occluded ? 0.0f : 1.0f;
	const float							OcclusionCertaintyFactor	= Lerp( 1.0f - m_OcclusionCertaintyFactor, 1.0f, OcclusionCertainty );

	OutCertainty													= DistanceCertaintyFactor * OcclusionCertaintyFactor;
	return true;
}

void WBCompEldSensorHearing::HandleNoise( WBEntity* const pNoiseEntity, const Vector& NoiseLocation, const float NoiseRadius, const float NoiseUpdateTime, const float ExpireTimeBonus ) const
{
	if( m_Paused )
	{
		return;
	}

	float Certainty = 0.0f;
	if( !GetNoiseCertainty( NoiseLocation, NoiseRadius, Certainty ) )
	{
		return;
	}

	WBEntity* const						pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompRodinKnowledge* const			pKnowledge	= GET_WBCOMP( pEntity, RodinKnowledge );
	ASSERT( pKnowledge );

	WBCompRodinKnowledge::TKnowledge&	Knowledge	= pKnowledge->UpdateEntity( pNoiseEntity, NoiseUpdateTime, ExpireTimeBonus );

	STATIC_HASHED_STRING( LastKnownLocation );
	Knowledge.SetVector( sLastKnownLocation, NoiseLocation );
	ASSERT( !NoiseLocation.IsZero() );

	STATIC_HASHED_STRING( LastHeardLocation );
	Knowledge.SetVector( sLastHeardLocation, NoiseLocation );

	STATIC_HASHED_STRING( LastHeardTime );
	Knowledge.SetFloat( sLastHeardTime, GetTime() );

	STATIC_HASHED_STRING( HearingCertainty );
	Knowledge.SetFloat( sHearingCertainty, Certainty );

	STATIC_HASHED_STRING( KnowledgeType );
	STATIC_HASHED_STRING( Target );
	Knowledge.SetHash( sKnowledgeType, sTarget );
}