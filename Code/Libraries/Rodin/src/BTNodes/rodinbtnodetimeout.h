#ifndef RODINBTNODETIMEOUT_H
#define RODINBTNODETIMEOUT_H

#include "rodinbtnodedecorator.h"
#include "wbparamevaluator.h"

class RodinBTNodeTimeout : public RodinBTNodeDecorator
{
public:
	RodinBTNodeTimeout();
	virtual ~RodinBTNodeTimeout();

	DEFINE_RODINBTNODE_FACTORY( Timeout );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus	Tick( float DeltaTime );

protected:
	WBParamEvaluator	m_TimeoutPE;
	float				m_NextCanExecuteTime;
};

#endif // RODINBTNODETIMEOUT_H