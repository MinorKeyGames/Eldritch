#include "core.h"
#include "midievent.h"
#include "idatastream.h"

MIDIEvent::MIDIEvent()
:	m_DeltaTime( 0 )
{
}

MIDIEvent::~MIDIEvent()
{
}

/*virtual*/ void MIDIEvent::Initialize( uint32 DeltaTime, uint8 EventCode )
{
	Unused( EventCode );

	m_DeltaTime = DeltaTime;
}

/*virtual*/ void MIDIEvent::Load( const IDataStream& Stream )
{
	Unused( Stream );
}

/*virtual*/ void MIDIEvent::Save( const IDataStream& Stream ) const
{
	Unused( Stream );
}