#ifndef WBACTIONRODINBLACKBOARDWRITE_H
#define WBACTIONRODINBLACKBOARDWRITE_H

#include "wbaction.h"
#include "wbparamevaluator.h"

class WBActionRodinBlackboardWrite : public WBAction
{
public:
	WBActionRodinBlackboardWrite();
	virtual ~WBActionRodinBlackboardWrite();

	DEFINE_WBACTION_FACTORY( RodinBlackboardWrite );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Execute();

private:
	HashedString		m_BlackboardKey;	// Config
	WBParamEvaluator	m_ValuePE;			// Config
};

#endif // WBACTIONRODINBLACKBOARDWRITE_H