#ifndef WBCOMPELDTHINKERPATROL_H
#define WBCOMPELDTHINKERPATROL_H

#include "wbcompeldthinker.h"

class WBCompEldThinkerPatrol : public WBCompEldThinker
{
public:
	WBCompEldThinkerPatrol();
	virtual ~WBCompEldThinkerPatrol();

	DEFINE_WBCOMP( EldThinkerPatrol, WBCompEldThinker );

	virtual void	Tick( float DeltaTime );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	HashedString	m_OutputBlackboardKey;
};

#endif // WBCOMPELDTHINKERPATROL_H