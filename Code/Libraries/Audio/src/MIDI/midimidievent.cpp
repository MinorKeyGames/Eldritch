#include "core.h"
#include "midimidievent.h"
#include "idatastream.h"

MIDIMIDIEvent::MIDIMIDIEvent()
:	m_MIDIEventType( 0 )
,	m_MIDIChannel( 0 )
{
}

MIDIMIDIEvent::~MIDIMIDIEvent()
{
}

/*virtual*/ void MIDIMIDIEvent::Initialize( uint32 DeltaTime, uint8 EventCode )
{
	MIDIEvent::Initialize( DeltaTime, EventCode );

	m_MIDIEventType = ( EventCode & 0xF0 ) >> 4;
	m_MIDIChannel = EventCode & 0x0F;
}

/*virtual*/ void MIDIMIDIEvent::Load( const IDataStream& Stream )
{
	MIDIEvent::Load( Stream );
}

/*virtual*/ void MIDIMIDIEvent::Save( const IDataStream& Stream ) const
{
	MIDIEvent::Save( Stream );
}