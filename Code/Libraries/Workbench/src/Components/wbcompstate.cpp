#include "core.h"
#include "wbcompstate.h"
#include "idatastream.h"
#include "wbevent.h"
#include "configmanager.h"
#include "reversehash.h"

WBCompState::WBCompState()
:	m_State()
{
}

WBCompState::~WBCompState()
{
}

/*virtual*/ void WBCompState::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( InitialState );
	m_State = ConfigManager::GetInheritedHash( sInitialState, HashedString::NullString, sDefinitionName );
}

/*virtual*/ void WBCompState::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( SetState );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sSetState )
	{
		STATIC_HASHED_STRING( NewState );
		const HashedString State = Event.GetHash( sNewState );

		SetState( State );
	}
}

/*virtual*/ void WBCompState::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Hash, State, m_State );
}

void WBCompState::Report() const
{
	Super::Report();

	const SimpleString State = ReverseHash::ReversedHash( m_State );
	PRINTF( WBPROPERTY_REPORT_PREFIX "Current State: %s\n", State.CStr() );
}

#define VERSION_EMPTY	0
#define VERSION_STATE	1
#define VERSION_CURRENT	1

uint WBCompState::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;						// Version
	Size += sizeof( HashedString );	// m_State

	return Size;
}

void WBCompState::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );
	Stream.WriteHashedString( m_State );
}

void WBCompState::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_STATE )
	{
		m_State = Stream.ReadHashedString();
	}
}