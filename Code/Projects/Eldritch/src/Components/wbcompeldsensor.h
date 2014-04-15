#ifndef WBCOMPELDSENSOR_H
#define WBCOMPELDSENSOR_H

#include "wbeldritchcomponent.h"

class WBCompEldSensor : public WBEldritchComponent
{
public:
	WBCompEldSensor();
	virtual ~WBCompEldSensor();

	virtual void	HandleEvent( const WBEvent& Event );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

protected:
	bool	m_Paused;	// Serialized
};

#endif // WBCOMPELDSENSOR_H