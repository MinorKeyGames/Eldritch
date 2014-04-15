#include "core.h"
#include "midinoteonevent.h"
#include "idatastream.h"

MIDINoteOnEvent::MIDINoteOnEvent()
:	m_Note( 0 )
,	m_Velocity( 0 )
{
}

MIDINoteOnEvent::~MIDINoteOnEvent()
{
}

/*virtual*/ void MIDINoteOnEvent::Load( const IDataStream& Stream )
{
	MIDIMIDIEvent::Load( Stream );

	m_Note = Stream.ReadUInt8();
	m_Velocity = Stream.ReadUInt8();
}

/*virtual*/ void MIDINoteOnEvent::Save( const IDataStream& Stream ) const
{
	MIDIMIDIEvent::Save( Stream );

	Stream.WriteUInt8( m_Note );
	Stream.WriteUInt8( m_Velocity );
}