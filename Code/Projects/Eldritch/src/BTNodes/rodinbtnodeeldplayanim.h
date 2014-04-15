#ifndef RODINBTNODEELDPLAYANIM_H
#define RODINBTNODEELDPLAYANIM_H

#include "rodinbtnode.h"
#include "iwbeventobserver.h"

class RodinBTNodeEldPlayAnim : public RodinBTNode, public IWBEventObserver
{
public:
	RodinBTNodeEldPlayAnim();
	virtual ~RodinBTNodeEldPlayAnim();

	DEFINE_RODINBTNODE_FACTORY( EldPlayAnim );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus	Tick( float DeltaTime );
	virtual void		OnStart();
	virtual void		OnFinish();

	// IWBEventObserver
	virtual void		HandleEvent( const WBEvent& Event );

private:
	HashedString	m_AnimationName;
	bool			m_Loop;

	bool			m_HasRequestedAnim;
};

#endif // RODINBTNODEELDPLAYANIM_H