#ifndef RODINBTNODEPLAYACTIONS_H
#define RODINBTNODEPLAYACTIONS_H

#include "rodinbtnode.h"
#include "array.h"

class WBAction;

class RodinBTNodePlayActions : public RodinBTNode
{
public:
	RodinBTNodePlayActions();
	virtual ~RodinBTNodePlayActions();

	DEFINE_RODINBTNODE_FACTORY( PlayActions );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus	Tick( float DeltaTime );

private:
	Array<WBAction*>	m_Actions;
};

#endif // RODINBTNODEPLAYACTIONS_H