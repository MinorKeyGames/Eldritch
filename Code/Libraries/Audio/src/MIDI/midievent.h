#ifndef MIDIEVENT_H
#define MIDIEVENT_H

class IDataStream;

class MIDIEvent
{
public:
	MIDIEvent();
	~MIDIEvent();

	virtual void Initialize( uint32 DeltaTime, uint8 EventCode );
	virtual void Load( const IDataStream& Stream );
	virtual void Save( const IDataStream& Stream ) const;

	uint32	GetDeltaTime() const { return m_DeltaTime; }

protected:
	uint32	m_DeltaTime;
};

#endif // MIDIEVENT_H