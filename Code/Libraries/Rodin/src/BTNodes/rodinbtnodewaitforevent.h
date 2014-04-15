#ifndef RODINBTNODEWAITFOREVENT_H
#define RODINBTNODEWAITFOREVENT_H

#include "rodinbtnode.h"
#include "wbrule.h"
#include "iwbeventobserver.h"

class RodinBTNodeWaitForEvent : public RodinBTNode, public IWBEventObserver
{
public:
	RodinBTNodeWaitForEvent();
	virtual ~RodinBTNodeWaitForEvent();

	DEFINE_RODINBTNODE_FACTORY( WaitForEvent );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus Tick( float DeltaTime );
	virtual void		OnStart();
	virtual void		OnFinish();

	// IWBEventObserver
	virtual void		HandleEvent( const WBEvent& Event );

protected:
	WBRule		m_Rule;
};

#endif // RODINBTNODEWAITFOREVENT_H