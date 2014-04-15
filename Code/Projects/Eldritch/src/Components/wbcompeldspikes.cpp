#include "core.h"
#include "wbcompeldspikes.h"
#include "wbcompeldtransform.h"
#include "wbcompeldcollision.h"
#include "wbeventmanager.h"
#include "configmanager.h"
#include "mathcore.h"

WBCompEldSpikes::WBCompEldSpikes()
:	m_SpeedThresholdSq( 0.0f )
,	m_CheckMovingDown( false )
,	m_RecentlyLandedThreshold( 0.0f )
{
}

WBCompEldSpikes::~WBCompEldSpikes()
{
}

/*virtual*/ void WBCompEldSpikes::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( SpeedThreshold );
	m_SpeedThresholdSq = Square( ConfigManager::GetInheritedFloat( sSpeedThreshold, 0.0f, sDefinitionName ) );

	STATICHASH( CheckMovingDown );
	m_CheckMovingDown = ConfigManager::GetInheritedBool( sCheckMovingDown, false, sDefinitionName );

	STATICHASH( RecentlyLandedThreshold );
	m_RecentlyLandedThreshold = ConfigManager::GetInheritedFloat( sRecentlyLandedThreshold, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldSpikes::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnTouched );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnTouched )
	{
		STATIC_HASHED_STRING( Touched );
		WBEntity* const pTouched = Event.GetEntity( sTouched );
		ASSERT( pTouched );

		HandleOnTouched( pTouched );
	}
}

void WBCompEldSpikes::HandleOnTouched( WBEntity* const pTouched )
{
	if( ShouldSendSpikedEvent( pTouched ) )
	{
		SendSpikedEvent( pTouched );
	}
}

bool WBCompEldSpikes::ShouldSendSpikedEvent( WBEntity* const pTouched )
{
	ASSERT( pTouched );

	WBCompEldTransform* const pTouchedTransform = pTouched->GetTransformComponent<WBCompEldTransform>();
	ASSERT( pTouchedTransform );

	WBCompEldCollision* const pTouchedCollision	= GET_WBCOMP( pTouched, EldCollision );
	ASSERT( pTouchedCollision );

	const Vector			Velocity	= pTouchedTransform->GetVelocity();

	if( m_CheckMovingDown && !pTouchedCollision->IsRecentlyLanded( m_RecentlyLandedThreshold ) )
	{
		static const Vector	kUp			= Vector( 0.0f, 0.0f, 1.0f );
		const float			VelDotUp	= Velocity.Dot( kUp );
		if( VelDotUp < 0.0f )
		{
			// Entity is moving down. Do spike.
			return true;
		}
	}

	const float				Speed2DSq	= Velocity.LengthSquared2D();
	if( Speed2DSq > m_SpeedThresholdSq )
	{
		// Entity is moving fast. Do spike.
		return true;
	}

	return false;
}

void WBCompEldSpikes::SendSpikedEvent( WBEntity* const pTouched )
{
	WB_MAKE_EVENT( OnSpiked, GetEntity() );
	WB_SET_AUTO( OnSpiked, Entity, Touched, pTouched );
	WB_DISPATCH_EVENT( GetEventManager(), OnSpiked, GetEntity() );
}