#ifndef WBACTIONUNQUEUEEVENT_H
#define WBACTIONUNQUEUEEVENT_H

// Requires event owner to have a VariableMap component for storing the event UID.

#include "wbaction.h"
#include "hashedstring.h"
#include "wbparamevaluator.h"

class WBActionUnqueueEvent : public WBAction
{
public:
	WBActionUnqueueEvent();
	virtual ~WBActionUnqueueEvent();

	DEFINE_WBACTION_FACTORY( UnqueueEvent );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	WBParamEvaluator	m_EventOwnerPE;
	HashedString		m_VariableMapTag;
};

#endif // WBACTIONUNQUEUEEVENT_H