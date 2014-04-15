#include "core.h"
#include "midinoteoffevent.h"
#include "idatastream.h"

MIDINoteOffEvent::MIDINoteOffEvent()
:	m_Note( 0 )
,	m_Velocity( 0 )
{
}

MIDINoteOffEvent::~MIDINoteOffEvent()
{
}

/*virtual*/ void MIDINoteOffEvent::Load( const IDataStream& Stream )
{
	MIDIMIDIEvent::Load( Stream );

	m_Note = Stream.ReadUInt8();
	m_Velocity = Stream.ReadUInt8();
}

/*virtual*/ void MIDINoteOffEvent::Save( const IDataStream& Stream ) const
{
	MIDIMIDIEvent::Save( Stream );

	Stream.WriteUInt8( m_Note );
	Stream.WriteUInt8( m_Velocity );
}