#include "core.h"
#include "midicontrollerevent.h"
#include "idatastream.h"

MIDIControllerEvent::MIDIControllerEvent()
:	m_ControllerType( 0 )
,	m_ControllerValue( 0 )
{
}

MIDIControllerEvent::~MIDIControllerEvent()
{
}

/*virtual*/ void MIDIControllerEvent::Load( const IDataStream& Stream )
{
	MIDIMIDIEvent::Load( Stream );

	m_ControllerType = Stream.ReadUInt8();
	m_ControllerValue = Stream.ReadUInt8();
}

/*virtual*/ void MIDIControllerEvent::Save( const IDataStream& Stream )
{
	MIDIMIDIEvent::Save( Stream );

	Stream.WriteUInt8( m_ControllerType );
	Stream.WriteUInt8( m_ControllerValue );
}