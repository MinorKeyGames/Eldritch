#ifndef MIDINOTEONEVENT_H
#define MIDINOTEONEVENT_H

#include "midimidievent.h"

class MIDINoteOnEvent : public MIDIMIDIEvent
{
public:
	MIDINoteOnEvent();
	~MIDINoteOnEvent();

	virtual void Load( const IDataStream& Stream );
	virtual void Save( const IDataStream& Stream ) const;

protected:
	uint8	m_Note;
	uint8	m_Velocity;
};

#endif // MIDINOTEONEVENT_H