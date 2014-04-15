#ifndef RODINBTNODECASTRESULT_H
#define RODINBTNODECASTRESULT_H

#include "rodinbtnodedecorator.h"
#include "wbparamevaluator.h"

class RodinBTNodeCastResult : public RodinBTNodeDecorator
{
public:
	RodinBTNodeCastResult();
	virtual ~RodinBTNodeCastResult();

	DEFINE_RODINBTNODE_FACTORY( CastResult );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus	Tick( float DeltaTime );

protected:
	WBParamEvaluator	m_ValuePE;
};

#endif // RODINBTNODECASTRESULT_H