#include "core.h"
#include "wbactioneldstopmotion.h"
#include "wbactionstack.h"
#include "Components/wbcompeldtransform.h"
#include "wbevent.h"
#include "angles.h"

WBActionEldStopMotion::WBActionEldStopMotion()
{
}

WBActionEldStopMotion::~WBActionEldStopMotion()
{
}

/*virtual*/ void WBActionEldStopMotion::Execute()
{
	WBAction::Execute();

	STATIC_HASHED_STRING( EventOwner );
	WBEntity* const pEntity = WBActionStack::Top().GetEntity( sEventOwner );

	if( pEntity )
	{
		WBCompEldTransform* const pTransform = pEntity->GetTransformComponent<WBCompEldTransform>();
		if( pTransform )
		{
			pTransform->SetGravity( 0.0f );
			pTransform->SetVelocity( Vector() );
			pTransform->SetAcceleration( Vector() );
			pTransform->SetRotationalVelocity( Angles() );
			pTransform->SetCanMove( false );
		}
	}
}