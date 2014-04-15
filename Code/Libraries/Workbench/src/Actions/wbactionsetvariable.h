#ifndef WBACTIONSETVARIABLE_H
#define WBACTIONSETVARIABLE_H

#include "wbaction.h"
#include "wbparamevaluator.h"

class WBActionSetVariable : public WBAction
{
public:
	WBActionSetVariable();
	virtual ~WBActionSetVariable();

	DEFINE_WBACTION_FACTORY( SetVariable );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	WBParamEvaluator	m_EntityPE;
	HashedString		m_VariableName;
	WBParamEvaluator	m_ValuePE;
};

#endif // WBACTIONSETVARIABLE_H