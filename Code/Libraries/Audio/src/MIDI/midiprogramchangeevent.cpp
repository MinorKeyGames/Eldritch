#include "core.h"
#include "midiprogramchangeevent.h"
#include "idatastream.h"

MIDIProgramChangeEvent::MIDIProgramChangeEvent()
:	m_ProgramNumber( 0 )
{
}

MIDIProgramChangeEvent::~MIDIProgramChangeEvent()
{
}

/*virtual*/ void MIDIProgramChangeEvent::Load( const IDataStream& Stream )
{
	MIDIMIDIEvent::Load( Stream );

	m_ProgramNumber = Stream.ReadUInt8();
}

/*virtual*/ void MIDIProgramChangeEvent::Save( const IDataStream& Stream ) const
{
	MIDIMIDIEvent::Save( Stream );

	Stream.WriteUInt8( m_ProgramNumber );
}