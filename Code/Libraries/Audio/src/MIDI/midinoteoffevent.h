#ifndef MIDINOTEOFFEVENT_H
#define MIDINOTEOFFEVENT_H

#include "midimidievent.h"

class MIDINoteOffEvent : public MIDIMIDIEvent
{
public:
	MIDINoteOffEvent();
	~MIDINoteOffEvent();

	virtual void Load( const IDataStream& Stream );
	virtual void Save( const IDataStream& Stream ) const;

protected:
	uint8	m_Note;
	uint8	m_Velocity;
};

#endif // MIDINOTEOFFEVENT_H