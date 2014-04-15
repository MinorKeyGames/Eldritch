#ifndef RODINBTNODEDECORATOR_H
#define RODINBTNODEDECORATOR_H

#include "rodinbtnode.h"

class RodinBTNodeDecorator : public RodinBTNode
{
public:
	RodinBTNodeDecorator();
	virtual ~RodinBTNodeDecorator();

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus Tick( float DeltaTime );
	virtual void		OnStart();
	virtual void		OnFinish();
	virtual void		OnChildCompleted( RodinBTNode* pChildNode, ETickStatus TickStatus );

	virtual void		Report( uint Depth );

protected:
	RodinBTNode*	m_Child;
	ETickStatus		m_ChildStatus;
};

#endif // RODINBTNODEDECORATOR_H