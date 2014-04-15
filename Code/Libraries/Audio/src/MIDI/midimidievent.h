#ifndef MIDIMIDIEVENT_H
#define MIDIMIDIEVENT_H

#include "midievent.h"

class MIDIMIDIEvent : public MIDIEvent
{
public:
	MIDIMIDIEvent();
	~MIDIMIDIEvent();

	virtual void Initialize( uint32 DeltaTime, uint8 EventCode );
	virtual void Load( const IDataStream& Stream );
	virtual void Save( const IDataStream& Stream ) const;

protected:
	uint8	m_MIDIEventType;
	uint8	m_MIDIChannel;
};

#endif // MIDIMIDIEVENT_H