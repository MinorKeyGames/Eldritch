#include "core.h"
#include "wbcompeldsensordamage.h"
#include "wbevent.h"
#include "Components/wbcomprodinknowledge.h"
#include "wbcompeldtransform.h"

WBCompEldSensorDamage::WBCompEldSensorDamage()
{
}

WBCompEldSensorDamage::~WBCompEldSensorDamage()
{
}

/*virtual*/ void WBCompEldSensorDamage::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnDamaged );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnDamaged )
	{
		STATIC_HASHED_STRING( Damager );
		WBEntity* const pDamager = Event.GetEntity( sDamager );
		ASSERT( pDamager );

		HandleDamage( pDamager );
	}
}

// TODO: Account for deferred damage like trap bolts.
// Maybe just make that a parameter of damage dealer, that it doesn't trigger sensor, because how could the AI know anything about it?
void WBCompEldSensorDamage::HandleDamage( WBEntity* const pDamager ) const
{
	XTRACE_FUNCTION;

	if( !pDamager )
	{
		return;
	}

	WBCompEldTransform* const			pTransform		= pDamager->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompRodinKnowledge* const			pKnowledge		= GET_WBCOMP( GetEntity(), RodinKnowledge );
	ASSERT( pKnowledge );

	WBCompRodinKnowledge::TKnowledge&	Knowledge		= pKnowledge->UpdateEntity( pDamager );

	STATIC_HASHED_STRING( RegardAsHostile );
	Knowledge.SetBool( sRegardAsHostile, true );

	STATIC_HASHED_STRING( IsDamager );
	Knowledge.SetBool( sIsDamager, true );

	STATIC_HASHED_STRING( LastKnownLocation );
	Knowledge.SetVector( sLastKnownLocation, pTransform->GetLocation() );
	ASSERT( !pTransform->GetLocation().IsZero() );

	STATIC_HASHED_STRING( KnowledgeType );
	STATIC_HASHED_STRING( Target );
	Knowledge.SetHash( sKnowledgeType, sTarget );
}