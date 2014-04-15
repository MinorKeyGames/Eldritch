#ifndef RODINBTNODEELDMOVETO_H
#define RODINBTNODEELDMOVETO_H

#include "rodinbtnode.h"

class RodinBTNodeEldMoveTo : public RodinBTNode
{
public:
	RodinBTNodeEldMoveTo();
	virtual ~RodinBTNodeEldMoveTo();

	DEFINE_RODINBTNODE_FACTORY( EldMoveTo );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus	Tick( float DeltaTime );
	virtual void		OnStart();
	virtual void		OnFinish();

private:
	enum EMoveToState
	{
		EMTS_Begin,
		EMTS_StartedMove,
	};

	HashedString	m_MoveTargetBlackboardKey;	// Config
	bool			m_Wander;					// Config
	float			m_WanderTargetDistance;		// Config
	float			m_ReachedThresholdMin;		// Config
	float			m_ReachedThresholdMax;		// Config
	float			m_FlyingDeflectionRadius;	// Config
	EMoveToState	m_MoveState;				// Transient
};

#endif // RODINBTNODEELDMOVETO_H