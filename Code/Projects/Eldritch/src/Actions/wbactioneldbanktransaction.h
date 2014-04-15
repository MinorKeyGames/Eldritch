#ifndef WBACTIONELDBANKTRANSACTION_H
#define WBACTIONELDBANKTRANSACTION_H

#include "wbaction.h"

class WBActionEldBankTransaction : public WBAction
{
public:
	WBActionEldBankTransaction();
	virtual ~WBActionEldBankTransaction();

	DEFINE_WBACTION_FACTORY( EldBankTransaction );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Execute();

private:
	int				m_Amount;
};

#endif // WBACTIONELDBANKTRANSACTION_H