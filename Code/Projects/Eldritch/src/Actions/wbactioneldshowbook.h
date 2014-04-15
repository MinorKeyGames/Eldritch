#ifndef WBACTIONELDSHOWBOOK_H
#define WBACTIONELDSHOWBOOK_H

#include "wbaction.h"
#include "simplestring.h"
#include "wbparamevaluator.h"

class WBActionEldShowBook : public WBAction
{
public:
	WBActionEldShowBook();
	virtual ~WBActionEldShowBook();

	DEFINE_WBACTION_FACTORY( EldShowBook );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Execute();

private:
	SimpleString		m_BookString;
	WBParamEvaluator	m_BookStringPE;
	bool				m_IsDynamic;
};

#endif // WBACTIONELDSHOWBOOK_H