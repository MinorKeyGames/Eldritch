#ifndef WBACTIONSELECTOR_H
#define WBACTIONSELECTOR_H

// Evaluates conditional PEs and executes the first action whose PE returns true.

#include "wbaction.h"
#include "array.h"
#include "wbparamevaluator.h"

class WBActionSelector : public WBAction
{
public:
	WBActionSelector();
	virtual ~WBActionSelector();

	DEFINE_WBACTION_FACTORY( Selector );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Execute();

private:
	struct SSelection
	{
		SSelection()
		:	m_ConditionPE()
		,	m_Action( NULL )
		{
		}

		WBParamEvaluator	m_ConditionPE;
		WBAction*			m_Action;
	};

	Array<SSelection>	m_Selections;
};

#endif // WBACTIONSELECTOR_H