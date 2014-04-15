#include "core.h"
#include "rodinbtnodewait.h"
#include "configmanager.h"
#include "wbentity.h"
#include "wbeventmanager.h"
#include "Components/wbcomprodinbehaviortree.h"

RodinBTNodeWait::RodinBTNodeWait()
:	m_TimePE()
,	m_TimerStarted( false )
,	m_EventHandle( 0 )
{
}

RodinBTNodeWait::~RodinBTNodeWait()
{
	WBEventManager* const pEventManager = GetEventManager();
	if( pEventManager )
	{
		pEventManager->UnqueueEvent( m_EventHandle );
	}
}

void RodinBTNodeWait::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( TimePE );
	m_TimePE.InitializeFromDefinition( ConfigManager::GetString( sTimePE, "", sDefinitionName ) );
}

RodinBTNode::ETickStatus RodinBTNodeWait::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	if( m_TimerStarted )
	{
		return ETS_Success;
	}
	else
	{
		WBParamEvaluator::SPEContext Context;
		Context.m_Entity = GetEntity();
		m_TimePE.Evaluate( Context );

		WB_MAKE_EVENT( OnWaitElapsed, NULL );
		m_EventHandle = WB_QUEUE_EVENT_DELAY( GetEventManager(), OnWaitElapsed, this, m_TimePE.GetFloat() );

		m_TimerStarted = true;

		// Go to sleep and wait for animation to complete.
		m_BehaviorTree->Sleep( this );
		return ETS_Running;
	}
}

void RodinBTNodeWait::OnStart()
{
	RodinBTNode::OnStart();

	m_TimerStarted = false;
}

void RodinBTNodeWait::OnFinish()
{
	RodinBTNode::OnFinish();

	GetEventManager()->UnqueueEvent( m_EventHandle );

	if( m_IsSleeping )
	{
		m_BehaviorTree->Wake( this );
	}
}

/*virtual*/ void RodinBTNodeWait::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	STATIC_HASHED_STRING( OnWaitElapsed );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnWaitElapsed )
	{
		if( m_IsSleeping )
		{
			m_BehaviorTree->Wake( this );
		}
	}
}