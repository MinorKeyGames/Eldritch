#ifndef MIDICONTROLLEREVENT_H
#define MIDICONTROLLEREVENT_H

#include "midimidievent.h"

class MIDIControllerEvent : public MIDIMIDIEvent
{
public:
	MIDIControllerEvent();
	~MIDIControllerEvent();

	virtual void Load( const IDataStream& Stream );
	virtual void Save( const IDataStream& Stream );

protected:
	uint8	m_ControllerType;
	uint8	m_ControllerValue;
};

#endif // MIDICONTROLLEREVENT_H