#ifndef WBCOMPELDSENSORMARKUP_H
#define WBCOMPELDSENSORMARKUP_H

#include "wbcompeldsensorpoll.h"

class WBCompEldSensorMarkup : public WBCompEldSensorPoll
{
public:
	WBCompEldSensorMarkup();
	virtual ~WBCompEldSensorMarkup();

	DEFINE_WBCOMP( EldSensorMarkup, WBCompEldSensorPoll );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	PollTick( const float DeltaTime ) const;

private:
	HashedString	m_Markup;
	float			m_Radius;
};

#endif // WBCOMPELDSENSORMARKUP_H