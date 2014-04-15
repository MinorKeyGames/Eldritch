#include "core.h"
#include "rodinbtnodedecorator.h"
#include "rodinbtnodefactory.h"
#include "Components/wbcomprodinbehaviortree.h"
#include "configmanager.h"

RodinBTNodeDecorator::RodinBTNodeDecorator()
:	m_Child( NULL )
,	m_ChildStatus( ETS_None )
{
}

RodinBTNodeDecorator::~RodinBTNodeDecorator()
{
	SafeDelete( m_Child );
}

void RodinBTNodeDecorator::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Child );
	const SimpleString ChildDef = ConfigManager::GetString( sChild, "", sDefinitionName );
	m_Child = RodinBTNodeFactory::Create( ChildDef, m_BehaviorTree );
	ASSERT( m_Child );
}

RodinBTNode::ETickStatus RodinBTNodeDecorator::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	if( m_ChildStatus == ETS_Running )
	{
		WARN;	// This case shouldn't happen if we put this task to sleep, so make sure!
		return ETS_Running;
	}

	if( m_ChildStatus == ETS_None )
	{
		// Start child running (first time Tick is called)
		m_BehaviorTree->Start( m_Child, this );
		m_ChildStatus = ETS_Running;
		m_BehaviorTree->Sleep( this );
		return ETS_Running;
	}
	else if( m_ChildStatus == ETS_Success )
	{
		return ETS_Success;
	}
	else
	{
		return ETS_Fail;
	}
}

void RodinBTNodeDecorator::OnStart()
{
	RodinBTNode::OnStart();

	m_ChildStatus = ETS_None;
}

void RodinBTNodeDecorator::OnFinish()
{
	RodinBTNode::OnFinish();

	// In case this is interrupted, clean up running subtask.
	if( m_ChildStatus == ETS_Running )
	{
		m_BehaviorTree->Stop( m_Child );
	}
}

void RodinBTNodeDecorator::OnChildCompleted( RodinBTNode* pChildNode, ETickStatus TickStatus )
{
	RodinBTNode::OnChildCompleted( pChildNode, TickStatus );

	ASSERT( pChildNode == m_Child );
	m_ChildStatus = TickStatus;

	m_BehaviorTree->Wake( this );
}

void RodinBTNodeDecorator::Report( uint Depth )
{
	RodinBTNode::Report( Depth );

	m_Child->Report( Depth + 1 );
}