#ifndef RODINBTNODEUSESTATMOD_H
#define RODINBTNODEUSESTATMOD_H

#include "rodinbtnodedecorator.h"

class RodinBTNodeUseStatMod : public RodinBTNodeDecorator
{
public:
	RodinBTNodeUseStatMod();
	virtual ~RodinBTNodeUseStatMod();

	DEFINE_RODINBTNODE_FACTORY( UseStatMod );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void		OnStart();
	virtual void		OnFinish();

protected:
	HashedString	m_StatModEvent;
};

#endif // RODINBTNODEUSESTATMOD_H