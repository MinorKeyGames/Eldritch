#include "core.h"
#include "midipitchbendevent.h"
#include "idatastream.h"

MIDIPitchBendEvent::MIDIPitchBendEvent()
:	m_PitchBendValue( 0 )
{
}

MIDIPitchBendEvent::~MIDIPitchBendEvent()
{
}

/*virtual*/ void MIDIPitchBendEvent::Load( const IDataStream& Stream )
{
	MIDIMIDIEvent::Load( Stream );

	const uint8 LSB = Stream.ReadUInt8();
	const uint8 MSB = Stream.ReadUInt8();

	m_PitchBendValue = ( LSB & 0x7F ) | ( ( MSB & 0x7F ) << 7 );
}

/*virtual*/ void MIDIPitchBendEvent::Save( const IDataStream& Stream ) const
{
	MIDIMIDIEvent::Save( Stream );

	const uint8 LSB = ( uint8 )( m_PitchBendValue & 0x007F );
	const uint8 MSB = ( uint8 )( ( m_PitchBendValue & 0x7F00 ) >> 7 );

	Stream.WriteUInt8( LSB );
	Stream.WriteUInt8( MSB );
}