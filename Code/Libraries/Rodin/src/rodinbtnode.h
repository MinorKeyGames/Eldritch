#ifndef RODINBTNODE_H
#define RODINBTNODE_H

#include "simplestring.h"

class WBCompRodinBehaviorTree;
class WBEntity;
class WBEventManager;

#define DEFINE_RODINBTNODE_FACTORY( type ) static class RodinBTNode* Factory() { return new RodinBTNode##type; }
typedef class RodinBTNode* ( *RodinBTNodeFactoryFunc )( void );

class RodinBTNode
{
public:
	enum ETickStatus
	{
		ETS_None,
		ETS_Fail,
		ETS_Running,
		ETS_Success
	};

	RodinBTNode();
	virtual ~RodinBTNode();

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus	Tick( float DeltaTime );
	virtual void		OnStart();
	virtual void		OnFinish();
	virtual void		OnChildCompleted( RodinBTNode* pChildNode, ETickStatus TickStatus );

	virtual void		Report( uint Depth );

	WBEntity*			GetEntity() const;
	float				GetTime() const;
	WBEventManager*		GetEventManager() const;
	SimpleString		GetName() const;

	WBCompRodinBehaviorTree*	m_BehaviorTree;	// Provided for behavior context--this is our scheduler, and we can get back to the entity through it
	bool						m_IsSleeping;

	// Cached name, for debugging purposes
	SimpleString			m_DefinitionName;
};

#endif // RODINBTNODE_H