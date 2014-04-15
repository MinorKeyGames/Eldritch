#ifndef WBCOMPELDSENSORDAMAGE_H
#define WBCOMPELDSENSORDAMAGE_H

#include "wbcompeldsensor.h"

class WBCompEldSensorDamage : public WBCompEldSensor
{
public:
	WBCompEldSensorDamage();
	virtual ~WBCompEldSensorDamage();

	DEFINE_WBCOMP( EldSensorDamage, WBCompEldSensor );

	virtual void	HandleEvent( const WBEvent& Event );

private:
	void	HandleDamage( WBEntity* const pDamager ) const;
};

#endif // WBCOMPELDSENSORDAMAGE_H