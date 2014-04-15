#ifndef RODINBTNODEELDTURNTOWARD_H
#define RODINBTNODEELDTURNTOWARD_H

#include "rodinbtnode.h"

class RodinBTNodeEldTurnToward : public RodinBTNode
{
public:
	RodinBTNodeEldTurnToward();
	virtual ~RodinBTNodeEldTurnToward();

	DEFINE_RODINBTNODE_FACTORY( EldTurnToward );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus	Tick( float DeltaTime );
	virtual void		OnStart();
	virtual void		OnFinish();

private:
	enum ETurnTowardState
	{
		ETTS_Begin,
		ETTS_StartedTurn,
	};

	HashedString		m_TurnTargetBlackboardKey;	// Config
	ETurnTowardState	m_TurnState;				// Transient
};

#endif // RODINBTNODEELDTURNTOWARD_H