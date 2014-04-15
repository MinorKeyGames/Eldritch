#ifndef WBACTIONSENDEVENT_H
#define WBACTIONSENDEVENT_H

#include "wbaction.h"
#include "hashedstring.h"
#include "array.h"
#include "wbparamevaluator.h"

class WBActionSendEvent : public WBAction
{
public:
	WBActionSendEvent();
	virtual ~WBActionSendEvent();

	DEFINE_WBACTION_FACTORY( SendEvent );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	struct SNamedParameter
	{
		HashedString		m_Name;
		WBParamEvaluator	m_ValuePE;
	};

	HashedString			m_EventName;
	bool					m_QueueEvent;
	bool					m_LogEvent;
	float					m_DispatchDelay;
	WBParamEvaluator		m_DispatchDelayPE;
	WBParamEvaluator		m_EventOwnerPE;
	WBParamEvaluator		m_RecipientPE;
	Array<SNamedParameter>	m_Parameters;

	// For tracking queued events in a var map
	HashedString			m_VariableMapTag;
};

#endif // WBACTIONSENDEVENT_H