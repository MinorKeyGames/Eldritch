#ifndef RODINBTNODELOG_H
#define RODINBTNODELOG_H

#include "rodinbtnode.h"
#include "simplestring.h"

class RodinBTNodeLog : public RodinBTNode
{
public:
	RodinBTNodeLog();
	virtual ~RodinBTNodeLog();

	DEFINE_RODINBTNODE_FACTORY( Log );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus Tick( float DeltaTime );

private:
	SimpleString	m_Message;			// Config
};

#endif // RODINBTNODELOG_H