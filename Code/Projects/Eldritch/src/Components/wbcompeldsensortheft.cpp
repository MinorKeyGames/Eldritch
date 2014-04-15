#include "core.h"
#include "wbcompeldsensortheft.h"
#include "wbeventmanager.h"
#include "Components/wbcomprodinknowledge.h"
#include "wbcompeldtransform.h"

WBCompEldSensorTheft::WBCompEldSensorTheft()
{
	STATIC_HASHED_STRING( OnTheft );
	GetEventManager()->AddObserver( sOnTheft, this );
}

WBCompEldSensorTheft::~WBCompEldSensorTheft()
{
	WBEventManager* const pEventManager = GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( OnTheft );
		pEventManager->RemoveObserver( sOnTheft, this );
	}
}

/*virtual*/ void WBCompEldSensorTheft::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnTheft );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnTheft )
	{
		STATIC_HASHED_STRING( Thief );
		WBEntity* const pThief = Event.GetEntity( sThief );

		HandleTheft( pThief );

		// Also, forward event to this entity for scripting to handle.
		{
			WB_MAKE_EVENT( OnTheftSensed, GetEntity() );
			WB_SET_AUTO( OnTheftSensed, Entity, Thief, pThief );
			WB_DISPATCH_EVENT( GetEventManager(), OnTheftSensed, GetEntity() );
		}
	}
}

void WBCompEldSensorTheft::HandleTheft( WBEntity* const pThief ) const
{
	WBCompEldTransform* const			pTransform		= pThief->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompRodinKnowledge* const			pKnowledge		= GET_WBCOMP( GetEntity(), RodinKnowledge );
	ASSERT( pKnowledge );

	WBCompRodinKnowledge::TKnowledge&	Knowledge		= pKnowledge->UpdateEntity( pThief );

	STATIC_HASHED_STRING( NeverExpire );
	Knowledge.SetBool( sNeverExpire, true );

	STATIC_HASHED_STRING( RegardAsHostile );
	Knowledge.SetBool( sRegardAsHostile, true );

	STATIC_HASHED_STRING( LastKnownLocation );
	Knowledge.SetVector( sLastKnownLocation, pTransform->GetLocation() );
	ASSERT( !pTransform->GetLocation().IsZero() );

	STATIC_HASHED_STRING( KnowledgeType );
	STATIC_HASHED_STRING( Target );
	Knowledge.SetHash( sKnowledgeType, sTarget );
}