#include "core.h"
#include "wbcompeldhard.h"
#include "idatastream.h"
#include "wbeventmanager.h"
#include "eldritchgame.h"

WBCompEldHard::WBCompEldHard()
:	m_Hard( false )
{
}

WBCompEldHard::~WBCompEldHard()
{
}

/*virtual*/ void WBCompEldHard::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( SetHard );
	STATIC_HASHED_STRING( PushPersistence );
	STATIC_HASHED_STRING( PullPersistence );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sSetHard )
	{
		m_Hard = true;

		// Broadcast event to everything with an EldHardListener component.
		WB_MAKE_EVENT( NotifyHardModeSet, NULL );
		WB_DISPATCH_EVENT( GetEventManager(), NotifyHardModeSet, NULL );
	}
	else if( EventName == sPushPersistence )
	{
		PushPersistence();
	}
	else if( EventName == sPullPersistence )
	{
		PullPersistence();
	}
}

void WBCompEldHard::PushPersistence() const
{
	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	STATIC_HASHED_STRING( Hard );
	Persistence.SetBool( sHard, m_Hard );
}

void WBCompEldHard::PullPersistence()
{
	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	STATIC_HASHED_STRING( Hard );
	m_Hard = Persistence.GetBool( sHard );
}

#define VERSION_EMPTY	0
#define VERSION_HARD	1
#define VERSION_CURRENT	1

uint WBCompEldHard::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;	// Version
	Size += 1;	// m_Hard

	return Size;
}

void WBCompEldHard::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );
	Stream.WriteBool( m_Hard );
}

void WBCompEldHard::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_HARD )
	{
		m_Hard = Stream.ReadBool();
	}
}