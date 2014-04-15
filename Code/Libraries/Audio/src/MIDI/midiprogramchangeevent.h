#ifndef MIDIPROGRAMCHANGEEVENT_H
#define MIDIPROGRAMCHANGEEVENT_H

#include "midimidievent.h"

class MIDIProgramChangeEvent : public MIDIMIDIEvent
{
public:
	MIDIProgramChangeEvent();
	~MIDIProgramChangeEvent();

	virtual void Load( const IDataStream& Stream );
	virtual void Save( const IDataStream& Stream ) const;

protected:
	uint8	m_ProgramNumber;
};

#endif // MIDIPROGRAMCHANGEEVENT_H