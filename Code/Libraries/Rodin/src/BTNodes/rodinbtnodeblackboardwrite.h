#ifndef RODINBTNODEBLACKBOARDWRITE_H
#define RODINBTNODEBLACKBOARDWRITE_H

#include "rodinbtnode.h"
#include "wbparamevaluator.h"

class RodinBTNodeBlackboardWrite : public RodinBTNode
{
public:
	RodinBTNodeBlackboardWrite();
	virtual ~RodinBTNodeBlackboardWrite();

	DEFINE_RODINBTNODE_FACTORY( BlackboardWrite );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus	Tick( float DeltaTime );

private:
	HashedString		m_BlackboardKey;	// Config
	WBParamEvaluator	m_ValuePE;			// Config
};

#endif // RODINBTNODEBLACKBOARDWRITE_H