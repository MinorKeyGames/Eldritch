#ifndef WBCOMPELDSENSORTHEFT_H
#define WBCOMPELDSENSORTHEFT_H

#include "wbcompeldsensor.h"

class WBCompEldSensorTheft : public WBCompEldSensor
{
public:
	WBCompEldSensorTheft();
	virtual ~WBCompEldSensorTheft();

	DEFINE_WBCOMP( EldSensorTheft, WBCompEldSensor );

	virtual void	HandleEvent( const WBEvent& Event );

private:
	void	HandleTheft( WBEntity* const pThief ) const;
};

#endif // WBCOMPELDSENSORTHEFT_H