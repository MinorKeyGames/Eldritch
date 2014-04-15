#ifndef MIDIPITCHBENDEVENT_H
#define MIDIPITCHBENDEVENT_H

#include "midimidievent.h"

class MIDIPitchBendEvent : public MIDIMIDIEvent
{
public:
	MIDIPitchBendEvent();
	~MIDIPitchBendEvent();

	virtual void Load( const IDataStream& Stream );
	virtual void Save( const IDataStream& Stream ) const;

protected:
	uint16	m_PitchBendValue;
};

#endif // MIDIPITCHBENDEVENT_H