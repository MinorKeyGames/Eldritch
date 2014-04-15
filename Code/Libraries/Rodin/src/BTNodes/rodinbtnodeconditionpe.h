#ifndef RODINBTNODECONDITIONPE_H
#define RODINBTNODECONDITIONPE_H

#include "rodinbtnode.h"
#include "wbparamevaluator.h"

class RodinBTNodeConditionPE : public RodinBTNode
{
public:
	RodinBTNodeConditionPE();
	virtual ~RodinBTNodeConditionPE();

	DEFINE_RODINBTNODE_FACTORY( ConditionPE );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus Tick( float DeltaTime );

private:
	WBParamEvaluator	m_ValuePE;			// Config
};

#endif // RODINBTNODECONDITIONPE_H