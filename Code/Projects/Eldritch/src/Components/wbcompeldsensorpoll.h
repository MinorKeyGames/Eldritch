#ifndef WBCOMPELDSENSORPOLL_H
#define WBCOMPELDSENSORPOLL_H

#include "wbcompeldsensor.h"

class WBCompEldSensorPoll : public WBCompEldSensor
{
public:
	WBCompEldSensorPoll();
	virtual ~WBCompEldSensorPoll();

	DEFINE_WBCOMP( EldSensorPoll, WBCompEldSensor );

	virtual void	Tick( float DeltaTime );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	PollTick( const float DeltaTime ) const;

private:
	bool	m_DoPoll;			// Config
	float	m_TickDeltaMin;		// Config
	float	m_TickDeltaMax;		// Config
	float	m_NextTickTime;		// Transient
};

#endif // WBCOMPELDSENSORPOLL_H