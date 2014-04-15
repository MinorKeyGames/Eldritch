#ifndef RODINBTNODESENDEVENT_H
#define RODINBTNODESENDEVENT_H

#include "rodinbtnode.h"
#include "wbparamevaluator.h"

class RodinBTNodeSendEvent : public RodinBTNode
{
public:
	RodinBTNodeSendEvent();
	virtual ~RodinBTNodeSendEvent();

	DEFINE_RODINBTNODE_FACTORY( SendEvent );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus	Tick( float DeltaTime );

private:
	struct SNamedParameter
	{
		HashedString		m_Name;
		WBParamEvaluator	m_ValuePE;
	};

	HashedString			m_EventName;
	bool					m_QueueEvent;
	float					m_DispatchDelay;
	WBParamEvaluator		m_RecipientPE;
	Array<SNamedParameter>	m_Parameters;
};

#endif // RODINBTNODESENDEVENT_H