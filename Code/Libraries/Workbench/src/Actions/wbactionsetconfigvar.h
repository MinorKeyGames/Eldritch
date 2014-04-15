#ifndef WBACTIONSETCONFIGVAR_H
#define WBACTIONSETCONFIGVAR_H

#include "wbaction.h"
#include "simplestring.h"
#include "wbparamevaluator.h"

class WBActionSetConfigVar : public WBAction
{
public:
	WBActionSetConfigVar();
	virtual ~WBActionSetConfigVar();

	DEFINE_WBACTION_FACTORY( SetConfigVar );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	SimpleString		m_VarContext;
	SimpleString		m_VarName;
	WBParamEvaluator	m_ValuePE;
};

#endif // WBACTIONSETCONFIGVAR_H