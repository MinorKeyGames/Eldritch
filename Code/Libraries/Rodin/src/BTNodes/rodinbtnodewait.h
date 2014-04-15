#ifndef RODINBTNODEWAIT_H
#define RODINBTNODEWAIT_H

#include "rodinbtnode.h"
#include "wbparamevaluator.h"
#include "wbeventmanager.h"

class RodinBTNodeWait : public RodinBTNode, public IWBEventObserver
{
public:
	RodinBTNodeWait();
	virtual ~RodinBTNodeWait();

	DEFINE_RODINBTNODE_FACTORY( Wait );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus Tick( float DeltaTime );
	virtual void		OnStart();
	virtual void		OnFinish();

	// IWBEventObserver
	virtual void		HandleEvent( const WBEvent& Event );

private:
	WBParamEvaluator	m_TimePE;		// Config
	bool				m_TimerStarted;
	TEventUID			m_EventHandle;
};

#endif // RODINBTNODEWAIT_H