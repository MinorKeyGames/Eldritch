#include "core.h"
#include "wbcompeldhardlistener.h"
#include "wbeventmanager.h"

WBCompEldHardListener::WBCompEldHardListener()
{
	RegisterForEvents();
}

WBCompEldHardListener::~WBCompEldHardListener()
{
	UnregisterForEvents();
}

void WBCompEldHardListener::RegisterForEvents()
{
	WBEventManager* const pEventManager = GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( NotifyHardModeSet );
		pEventManager->AddObserver( sNotifyHardModeSet, this, NULL );
	}
}

void WBCompEldHardListener::UnregisterForEvents()
{
	WBEventManager* const pEventManager = GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( NotifyHardModeSet );
		pEventManager->RemoveObserver( sNotifyHardModeSet, this, NULL );
	}
}

/*virtual*/ void WBCompEldHardListener::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( NotifyHardModeSet );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sNotifyHardModeSet )
	{
		WB_MAKE_EVENT( OnSetHardMode, GetEntity() );
		WB_DISPATCH_EVENT( GetEventManager(), OnSetHardMode, GetEntity() );
	}
}