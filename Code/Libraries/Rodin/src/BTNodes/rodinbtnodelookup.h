#ifndef RODINBTNODELOOKUP_H
#define RODINBTNODELOOKUP_H

#include "rodinbtnodedecorator.h"

class RodinBTNodeLookup : public RodinBTNodeDecorator
{
public:
	RodinBTNodeLookup();
	virtual ~RodinBTNodeLookup();

	DEFINE_RODINBTNODE_FACTORY( Lookup );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
};

#endif // RODINBTNODELOOKUP_H