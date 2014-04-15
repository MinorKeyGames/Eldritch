#include "core.h"
#include "wbcompeldthinker.h"
#include "wbevent.h"

WBCompEldThinker::WBCompEldThinker()
{
}

WBCompEldThinker::~WBCompEldThinker()
{

}

/*virtual*/ void WBCompEldThinker::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	WBEldritchComponent::HandleEvent( Event );

	STATIC_HASHED_STRING( TickThinkers );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sTickThinkers )
	{
		Tick( 0.0f );
	}
}