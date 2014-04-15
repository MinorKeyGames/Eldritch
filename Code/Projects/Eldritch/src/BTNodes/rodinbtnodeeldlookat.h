#ifndef RODINBTNODEELDLOOKAT_H
#define RODINBTNODEELDLOOKAT_H

#include "rodinbtnode.h"

class RodinBTNodeEldLookAt : public RodinBTNode
{
public:
	RodinBTNodeEldLookAt();
	virtual ~RodinBTNodeEldLookAt();

	DEFINE_RODINBTNODE_FACTORY( EldLookAt );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus	Tick( float DeltaTime );

private:
	HashedString	m_LookTargetBlackboardKey;	// Config
};

#endif // RODINBTNODEELDLOOKAT_H