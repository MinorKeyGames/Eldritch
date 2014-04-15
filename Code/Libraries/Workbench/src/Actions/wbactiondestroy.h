#ifndef WBACTIONDESTROY_H
#define WBACTIONDESTROY_H

#include "wbaction.h"
#include "wbparamevaluator.h"

class WBActionDestroy : public WBAction
{
public:
	WBActionDestroy();
	virtual ~WBActionDestroy();

	DEFINE_WBACTION_FACTORY( Destroy );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Execute();

private:
	WBParamEvaluator	m_DestroyPE;
};

#endif // WBACTIONDESTROY_H