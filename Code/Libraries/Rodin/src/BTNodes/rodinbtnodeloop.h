#ifndef RODINBTNODELOOP_H
#define RODINBTNODELOOP_H

#include "rodinbtnodedecorator.h"

class RodinBTNodeLoop : public RodinBTNodeDecorator
{
public:
	RodinBTNodeLoop();
	virtual ~RodinBTNodeLoop();

	DEFINE_RODINBTNODE_FACTORY( Loop );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus Tick( float DeltaTime );

protected:
	bool				m_CanFail;
	bool				m_CanSucceed;
	float				m_LastTickTime;
};

#endif // RODINBTNODELOOP_H