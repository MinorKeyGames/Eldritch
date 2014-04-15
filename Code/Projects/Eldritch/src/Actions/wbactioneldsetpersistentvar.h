#ifndef WBACTIONELDSETPERSISTENTVAR_H
#define WBACTIONELDSETPERSISTENTVAR_H

#include "wbaction.h"
#include "hashedstring.h"
#include "wbparamevaluator.h"

class WBActionEldSetPersistentVar : public WBAction
{
public:
	WBActionEldSetPersistentVar();
	virtual ~WBActionEldSetPersistentVar();

	DEFINE_WBACTION_FACTORY( EldSetPersistentVar );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Execute();

private:
	HashedString		m_Key;
	WBParamEvaluator	m_ValuePE;
};

#endif // WBACTIONELDSETPERSISTENTVAR_H